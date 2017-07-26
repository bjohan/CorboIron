#include <LiquidCrystal_I2C.h>
extern "C"
{
#include "uart_command_lib.h"
#include "corbomite.h"
}


#define SET_HEATER_TIP3 (PORTD |= (1 << 4))
#define CLEAR_HEATER_TIP3 (PORTD &= ~(1 << 4))

#define SET_HEATER_TIP2 (PORTD |= (1 << 5))
#define CLEAR_HEATER_TIP2 (PORTD &= ~(1 << 5))

#define SET_HEATER_TIP1 (PORTD |= (1 << 6))
#define CLEAR_HEATER_TIP1 (PORTD &= ~(1 << 6))

#define SET_HEATER_TIP0 (PORTD |= (1 << 7))
#define CLEAR_HEATER_TIP0 (PORTD &= ~(1 << 7))

#define encoder0PinA 2
#define encoder0PinB 3
volatile int encoder0Pos = 0;

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

volatile uint8_t skip_samp = 64;
volatile uint8_t over_samp = 64;
volatile int sampCount = 0;

uint8_t cursorChannel = 0;
uint8_t channelIsSelected = 0;

struct tipState {
	uint16_t adc_value;
	uint16_t adc_value_target;
	uint8_t pwm_value;
	float temperature;
	float temperature_target;
};


volatile struct tipState ts[4] = {{0, 0, 0, 0, 300}, {0, 0, 0, 0, 250}, {0, 0, 0, 0, 250}, {0, 0, 0, 0, 250} };

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
extern "C" {
void setSetPoint( int32_t temp)
{
    setPoint = temp;
    ts[0].temperature_target = temp;
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


ANA_OUT("setPoint", "C", "0", "500", 0, 500, setSetPoint ,setPointTemperature);
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

void doEncoderA(){

  // look for a low-to-high on channel A
  if (digitalRead(encoder0PinA) == HIGH) { 

    // check channel B to see which way encoder is turning
    if (digitalRead(encoder0PinB) == LOW) {  
      encoder0Pos = encoder0Pos + 1;         // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  else   // must be a high-to-low edge on channel A                                       
  { 
    // check channel B to see which way encoder is turning  
    if (digitalRead(encoder0PinB) == HIGH) {   
      encoder0Pos = encoder0Pos + 1;          // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  //Serial.println (encoder0Pos, DEC);          
  // use for debugging - remember to comment out

}

void doEncoderB(){

  // look for a low-to-high on channel B
  if (digitalRead(encoder0PinB) == HIGH) {   

    // check channel A to see which way encoder is turning
    if (digitalRead(encoder0PinA) == HIGH) {  
      encoder0Pos = encoder0Pos + 1;         // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;         // CCW
    }
  }

  // Look for a high-to-low on channel B

  else { 
    // check channel B to see which way encoder is turning  
    if (digitalRead(encoder0PinA) == LOW) {   
      encoder0Pos = encoder0Pos + 1;          // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }

} 

void setupAdc(){
	ADCSRA = 1<< ADEN | 1<<ADIE | 1<<ADPS2 | 1 << ADPS1; //Enable ADC, interrupts and prescale adcclk by 64
	ADMUX = 1<<REFS0 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1; //Set REFS to 01 which is the AVCC pin
	sei();
	ADCSRA |= 1<<ADSC; //Start first conversion
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

    pinMode(encoder0PinA, INPUT); 
    pinMode(encoder0PinB, INPUT); 
    digitalWrite(encoder0PinA, HIGH);
    digitalWrite(encoder0PinB, HIGH);

    // encoder pin on interrupt 0 (pin 2)
    attachInterrupt(0, doEncoderA, CHANGE);

    // encoder pin on interrupt 1 (pin 3)
    attachInterrupt(1, doEncoderB, CHANGE);  


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


ISR(ADC_vect){
	static uint8_t ch_read = 0;
	static uint8_t samps = 0;
	static uint32_t current_value = 0;
	volatile struct tipState *ct;
	uint8_t c;
 	samps++;
	if(samps > skip_samp){
  		current_value += ADCL;
        current_value += (ADCH << 8);
    }
	if(samps >= over_samp+skip_samp){
		//adc_values[ch_read] = current_value;
		ts[ch_read].adc_value =  (current_value)>>6;
		current_value = 0;
  		samps = 0;
		ch_read++;
		ch_read = ch_read & 0x3;
		ADMUX = (ADMUX & 0xF0) | ch_read;
		if(ch_read == 0)
			sampCount++;
  	}

	ct = &ts[0];
	for(c = 0 ; c < 4 ; c++){
		if(c == ch_read){
			switch(c){
				case 0: CLEAR_HEATER_TIP0; break;
				case 1: CLEAR_HEATER_TIP1; break;
				case 2: CLEAR_HEATER_TIP2; break;
				case 3: CLEAR_HEATER_TIP3; break;
			}
		} else {
			if(samps < ct->pwm_value){
				switch(c){
					case 0: SET_HEATER_TIP0; break;
					case 1: SET_HEATER_TIP1; break;
					case 2: SET_HEATER_TIP2; break;
					case 3: SET_HEATER_TIP3; break;
				}
			}else{
				switch(c){
					case 0: CLEAR_HEATER_TIP0; break;
					case 1: CLEAR_HEATER_TIP1; break;
					case 2: CLEAR_HEATER_TIP2; break;
					case 3: CLEAR_HEATER_TIP3; break;
				}
			}
		}
		ct++;
  	}
  	ADCSRA |= 1<<ADSC;
}


#define K_t 0.0000160
#define M_t (-0.00096)

#define K_a 1200.0
#define M_a (-1.1)

#define K_c (1023.0/4.98)
#define M_c (-6.0)


#define K_tot (K_t*K_a*K_c)
#define M_tot (M_t*K_a*K_c + M_a*K_c + M_c)
 
float amp(float x){
	return x*1200.0-1.1;
}

float adc(float x){
	return x*1023.0/4.98 - 6.0;
}

float tip(float x){
	return x*0.0000160-0.00096;
}


float complete(float c){
    return (((c+13.0)/1.03)*K_tot+M_tot);
}

float inv_complete(float c){
    return ((c-M_tot)/K_tot)*1.03-13.0;
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

float c_to_code(float c){
	return adc(amp(tip((c+13.0)/1.03)));
}


void cursorLogic(){
    static uint8_t lastButtonState;
    static uint8_t start;
    int newT;
    uint8_t buttonState = digitalRead(8);
    if(channelIsSelected == 0){
        cursorChannel = (encoder0Pos >> 2)&0x03;
    } else {
        newT = start + encoder0Pos;
        if(newT < 120){
            newT = 120;
            start ;
            encoder0Pos = newT-start;
        }if(newT > 400){
            newT = 400;
            encoder0Pos = newT-start;
        }
        ts[cursorChannel].temperature_target = newT;
    

    }
    if(buttonState != lastButtonState){
            if(buttonState != 0){
                if(channelIsSelected == 0){
                    channelIsSelected = 1;
                    start = ts[cursorChannel].temperature_target;
                    encoder0Pos = 0;
                }else{
	                ts[cursorChannel].adc_value_target = complete(ts[cursorChannel].temperature_target);
                    channelIsSelected = 0;
                    encoder0Pos=cursorChannel << 2;
                }
            }
    }
  
    lastButtonState = buttonState;
}


void displayChannel(uint8_t ch, float setPoint, float actual, float power)
{ //                      01234567890123456789
  //const char templ[] = "0>383C(383.1C) 30.3W";
  char toPrint2[64];
  if(ch == cursorChannel){
      if(channelIsSelected == 0)
          sprintf(toPrint2, "%d>%03dC(000.0C) 00.0W", ch, int(setPoint));
      else
          sprintf(toPrint2, "%d=%03dC(000.0C) 00.0W", ch, int(setPoint));
  }
  else
      sprintf(toPrint2, "%d %03dC(000.0C) 00.0W", ch, int(setPoint));
  ftoa(toPrint2+7, actual, 3,1);
  ftoa(toPrint2+15, power, 2,1);
  toPrint2[20]='\0';
  lcd.setCursor(0,ch);
  lcd.print(toPrint2);

}

float regulateChannel(float target, float current){
	return (target - current)*3;
}

void controlChannel(volatile struct tipState *ts){
	int error = ts->adc_value_target - ts->adc_value;
	if(error < 0)
		error = 0;
    if(error > 128)
        error = 128;
	ts->pwm_value = error;
} 

void printControlChannel(volatile struct tipState *ts){
	Serial.print("tadc "); Serial.print(ts->adc_value_target);
	Serial.print(" cadc "); Serial.print(ts->adc_value);
	Serial.print(" pwm "); Serial.print(ts->pwm_value);
	//Serial.print(" temp "); Serial.print(ts->temperature);
    //&Serial.println("");
}

void loop()
{
	uint16_t ch;
	float target = 190;
	//float temp = 0;
    cursorLogic();
	static long rate = 0;
        for(ch = 0 ; ch < 1 ; ch ++){
		//temp = ((float) ts[ch].adc_value)/64.0;// readAdc(ch);
		//ts[ch].temperature = code_to_c(ts[ch].adc_value);
		ts[ch].temperature = inv_complete(ts[ch].adc_value);
		/*if(ts[ch].temperature_target > 0){
	                //ts[ch].adc_value_target = c_to_code(ts[ch].temperature_target);
	                ts[ch].adc_value_target = complete(ts[ch].temperature_target);
			ts[ch].temperature_target = -ts[ch].temperature_target;
		}*/
		controlChannel(&ts[ch]);
		/*if(ts[ch].temperature < target)
			ts[ch].pwm_value = 70;
		else
			ts[ch].pwm_value = 0;*/
                //powerFactors[ch] = regulateChannel(target, ts[ch].temperature);
        }

        for(ch = 0 ; ch < 4 ; ch ++){
		//ts[ch].adc_value_target = c_to_code(ts[ch].temperature)*64;
                displayChannel(ch, int(ts[ch].temperature_target), ts[ch].temperature, ts[ch].pwm_value); 
                //displayChannel(ch, 1, 2, 3); 
        }
	
	//setHeater(0, 1);
	//digitalWrite(7, HIGH);
	//transmitAnalogIn(&temperature, temperatures[0]);
	delay(10);
        commandLine();
	printControlChannel(&ts[0]);
	//Serial.print("Ch0: "); Serial.print(ts[0].adc_value); 
    Serial.print(" Enc "); Serial.print (encoder0Pos >> 2, DEC);
	Serial.print(" Iteration time "); Serial.print(micros()-rate); 
	Serial.print(" samps: "); Serial.print(sampCount);Serial.println("");
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
 
