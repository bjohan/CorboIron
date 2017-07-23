#include <LiquidCrystal_I2C.h>
extern "C"
{
#include "uart_command_lib.h"
#include "corbomite.h"
}

#define ADC_VOLT 5.0/1023.0;
#define TYPE_K_COEFF 0.000041
//#define GAIN (1.0+2200000.0/35700.0)
#define GAIN 950.0
#define ADC_TO_C TYPE_K_COEFF*ADC_VOLT/GAIN
int tempPins[] = {A0, A1, A6, A7};
int heaterPins[] = {7, 6, 5, 4};
int ch1TempPin = A0;
int chargePumpPin = 9;
int heaterPin = 7;
int heatLed = 13;
int heaterLevel = 200;
int rawAdcValue = 1024;
int setPoint = 0;
float sP;
float sI;
float sD;


LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
extern "C" {
void setSetPoint( int32_t temp)
{
    setPoint = temp;
}

void setPParameter( int32_t temp)
{
    sP = (float) temp/1000.0;
}

void setIParameter( int32_t temp)
{
    sI = (float) temp/1000.0;
}

void setDParameter( int32_t temp)
{
    sD = (float) temp/1000.0;
}


ANA_OUT("setPoint", "C", "0", "300", 0, 300, setSetPoint ,setPointTemperature);
ANA_OUT("setP", "C", "0", "3", 0, 3000, setPParameter ,setPWidget);
ANA_OUT("setI", "C", "0", "3", 0, 3000, setIParameter ,setIWidget);
ANA_OUT("setD", "C", "0", "3", 0, 3000, setDParameter ,setDWidget);
ANA_IN("temperature", "C", "0", "1023", 0, 1023, temperature);
ANA_IN("power", "W", "0", "40", 0, 255, powerWidget);
ANA_IN("ppower", "W", "0", "40", 0, 4000, pPowerWidget); 
ANA_IN("ipower", "W", "0", "40", 0, 4000, iPowerWidget); 
ANA_IN("dpower", "W", "0", "40", 0, 4000, dPowerWidget); 


const CorbomiteEntry last PROGMEM = {LASTTYPE, "", 0};
const EventData initEvent PROGMEM = {registeredEntries};

const CorbomiteEntry initcmd PROGMEM = 
	{EVENT_OUT, internalId, (CorbomiteData*)&initEvent};

const CorbomiteEntry * const entries[] PROGMEM = {
	&setPointTemperature,
        &setPWidget,
        &setIWidget,
        &setDWidget,
        &temperature,
        &powerWidget,
        &pPowerWidget,
        &iPowerWidget,
        &dPowerWidget,
	&initcmd, &last
};


}

void initLcd(){
	lcd.begin(20,4);
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print("Welcome to CorboIron");
  
        lcd.setCursor(0,2);
        lcd.print("Version 0.000");

        lcd.setCursor(0,3);
        for(int i = 0 ; i < 20 ; i++){
                lcd.print('-');
                delay(50);
        }
        lcd.setCursor(0,3);
        for(int i = 0 ; i < 20 ; i++){
                lcd.print(' ');
                delay(50);
        }

}





void setupAdc(){
	ADCSRA = 1<< ADEN | 1<<ADIE | 1<<ADPS2 | 1 << ADPS1; //Enable ADC, interrupts and prescale adcclk by 64
	ADMUX = 1<<REFS0 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1; //Set REFS to 01 which is the AVCC pin
	sei();
	ADCSRA |= 1<<ADSC; //Start first conversion
}


// Value to store analog result
volatile int adc_values[4] = {0,0,0,0};
volatile uint16_t pwm_values[4] = {0,0,0,0};
volatile uint8_t ch = 0;
volatile uint8_t samps = 0;
volatile uint8_t skip_samp = 15;
volatile uint8_t over_samp = 64;
volatile int sampCount = 0;
ISR(ADC_vect){
  uint8_t c;
  samps++;
  if(samps > skip_samp)
  	adc_values[ch] += ADCL | (ADCH << 8);
  
  if(samps >= over_samp+skip_samp){
  	samps = 0;
	ch++;
	ch = ch & 0x3;
	if(ch == 0)
		sampCount++;
  }
  for(c = 0 ; c < 4 ; c++){
	if(c == ch){
		digitalWrite(heaterPins[c], LOW);
	} else {
		if(samps < pwm_values[c])
			digitalWrite(heaterPins[c], HIGH);
		else
			digitalWrite(heaterPins[c], LOW);
	}
  }
  //analogVal = ADCL | (ADCH << 8);
  //analogVal++; 
  ADCSRA |= 1<<ADSC;
}


void setup()
{
        pinMode(ch1TempPin, INPUT);
	pinMode(chargePumpPin, OUTPUT);
        Serial.begin(115200);
        analogReference(DEFAULT);
        analogWrite(chargePumpPin, 127);
	initLcd();
	//digitalWrite(7, LOW);
	pinMode(7, OUTPUT);
        setupAdc();
}


float readAdc(int ch)
{
  int val;
  int valAcc = 0;
  if(ch > 3)
	return 0;
  //analogWrite(heaterPin, 0);
  for(int i = 0 ; i < 1 ; i++){
    val = analogRead(tempPins[ch]);
    valAcc += val;
  }
  //analogWrite(heaterPin, heaterLevel);
  rawAdcValue = val;
  return float(valAcc)/1.0;
}

float inv_adc(float x){
	return (x+6.0)/(1023.0/4.98);
}

float inv_amp(float x){
	return (x+1.1)/1200.0;
}

float inv_tip(float x){
	return (x+0.00096)/0.0000160-6.0;
}

float code_to_c(float c){
	return inv_tip(inv_amp(inv_adc(c)))*1.03-13.0;
}

void setHeater(uint8_t ch, uint8_t state){
	int pin;
	switch(ch){
		case 0:
			pin = 7;
		default:
			return;
	}
	if(state == 0)
		digitalWrite(pin, LOW);
	else
		digitalWrite(pin, HIGH);
}

int ftoa(char *buf, float remainder, int i, int d)
{
        unsigned int cont = 1;
        unsigned int bufPos = 0;
        int mag;
        float divisor, divident, roundup;
        
        if(remainder < 0){
                buf[bufPos++]='-';
                remainder = - remainder;
        }

        roundup = 0.5*pow(10, 0-float(d));
        remainder += roundup;
        mag = floor(log10(fabsf(remainder)));
        mag = mag < i-1 ? i-1 : mag;
        while(cont){
                divisor = pow(10, mag);
                divident = floor(remainder/divisor);
                remainder = remainder-divident*divisor;
                
                if(mag == -1)
                        buf[bufPos++]='.';
                buf[bufPos++]='0'+divident;
                mag = mag -1;
                if (mag < -d){
                        cont = 0;
                }
                
        }
        return bufPos;
}


void displayChannel(uint8_t ch, float setPoint, float actual, float power)
{ //                      01234567890123456789
  //const char templ[] = "0>383C(383.1C) 30.3W";
  char toPrint2[64];
  sprintf(toPrint2, "%d>%03dC(000.0C) 00.0W", ch, int(setPoint));
  ftoa(toPrint2+7, actual, 3,1);
  ftoa(toPrint2+15, power, 2,1);
  toPrint2[20]='\0';
  lcd.setCursor(0,ch);
  lcd.print(toPrint2);

}

float regulateChannel(float target, float current){
	return (target - current)*3;
}

void loop()
{
	uint8_t ch;
        float temperatures[4];
        float powerFactors[4];
        int milliseconds[4]; 
	float target = 190;
	float temp = 0;
	static long rate = 0;
	unsigned long t0 = micros();
	unsigned long ts;
	unsigned long conv = 0;
	unsigned long corr = 0;
        /*for(ch = 0 ; ch < 4 ; ch ++){
		ts = micros();
		temp = readAdc(ch);
		conv += micros()-ts;
		
		ts = micros();
                temperatures[ch] = code_to_c(temp);
		corr += micros()-ts;

                powerFactors[ch] = regulateChannel(target, temperatures[ch]);
        }*/

        /*for(ch = 0 ; ch < 4 ; ch ++){
                displayChannel(ch, int(target), temperatures[ch], powerFactors[ch]); 
        }*/
	
	//setHeater(0, 1);
	//digitalWrite(7, HIGH);
	//transmitAnalogIn(&temperature, temperatures[0]);
	delay(10);
        commandLine();
	Serial.print("Time for adc conversion ");Serial.print(conv);
	Serial.print(". Time for correction "); Serial.print(corr);
	Serial.print("Iteration time "); Serial.print(micros()-rate) ; 
	Serial.print(" aval: "); Serial.print(sampCount);Serial.println("");
	rate = micros();
}

void platformSerialWrite(const char *buf, uint16_t len)
{
    Serial.write((uint8_t *)buf, len);
}

void serialEvent()
{
    while(Serial.available()){
        addCharToBuffer(Serial.read());
    }
}
 
