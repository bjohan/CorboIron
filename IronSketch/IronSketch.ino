#include <EEPROM.h>
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

enum buttonAction{NONE,PRESS, LONG_PRESS};

volatile int encoder0Pos = 0;

uint32_t vOffset[4] = {uint32_t(1.538*65536), uint32_t(0.013*65536), uint32_t(0.750*65536), uint32_t(0.154*65536)};
uint32_t gain[4] = {uint32_t(525)<<16, uint32_t(525)<<16, uint32_t(525)<<16, uint32_t(525)<<16};
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

struct menuState {
	uint8_t pressCount;
	uint8_t cursorChannel;
	uint8_t channelIsSelected;
};

struct tipState {
	uint32_t adc_value;
	uint32_t adc_value_target;
	uint8_t pwm_value;
	unsigned int temperature;
	unsigned int temperature_target;
};


//table in 16.16 for converting celcius to mV. One point every 32 C
const uint32_t tToV[19] = {0, 21430, 45377, 71631, 100008, 130286, 162293, 195861, 230818, 267059, 304349, 342688, 381852, 421737, 462291, 503382, 544932, 586901, 629159};

//table in 16.16 for converting mV to celcius. One point every 0.5 mV
const uint32_t vToT[21] =  {0, 3114410, 5820835, 8266898, 10538541, 12681425, 14725620, 16692405, 18593499, 20447232, 22253590, 24026343, 25767091, 27484160, 29178644, 30855147, 32513138, 34160640, 35797002, 37423651, 39043960};

volatile struct tipState ts[4] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} };

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

uint32_t millivoltsToCelcius(uint32_t mv){
	uint8_t idx = mv>>15;
	if(idx >= 20) return (uint32_t) 1000<<16;
	uint32_t base = vToT[idx];
	uint32_t delta = vToT[idx+1]-base;
	uint32_t frac = ((mv&((uint32_t)0x7fff)));
	uint32_t prod = (delta>>7)*(frac>>8);
	//Serial.print("Base ");Serial.print(base/65536.0);Serial.print(" delta ");Serial.print(delta/65536.0);
	//Serial.print(" frac ");Serial.print( (frac)/32768.0); Serial.print(" prod " ); Serial.println(prod/65536.0);
	//return base+(((mv&0x7FFF)*delta)>>14);
	return base+prod;
}

uint32_t celciusToMillivolts(uint32_t c){
	uint8_t idx = c >> (16+5);
	if(idx >=18) return (uint32_t)10<< 16;
	uint32_t base = tToV[idx];
	uint32_t delta = tToV[idx+1]-base;
	uint32_t frac = c&0x1fffff;
	uint32_t prod = (delta>>8)*(frac>>13);
	//Serial.print("Base ");Serial.print(base/65536.0);Serial.print(" delta ");Serial.print(delta/65536.0);
	//Serial.print(" frac ");Serial.print( (frac>>5)/65536.0); Serial.print(" prod " ); Serial.println(prod/65536.0);
	return base+prod;
}

uint32_t clamp(int32_t low, int32_t val, int32_t high){
	if(val < low) return low;
	if(val > high) return high;
	return val;
}

void initLcd(){
	lcd.begin(20,4);
	lcd.backlight();
	lcd.setCursor(0,0);
	/*lcd.print("Welcome to CorboIron");

	lcd.setCursor(0,2);
	lcd.print("Version 0.000");

	lcd.setCursor(0,3);
	for(int i = 0 ; i < 20 ; i++){
		lcd.print('-');
		delay(10);
	}
	lcd.setCursor(0,3);
	for(int i = 0 ; i < 20 ; i++){
		lcd.print(' ');
		delay(10);
	}
*/
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
#define K_t 0.0000160
#define M_t (-0.00096)

#define K_a 1200.0
#define M_a (-1.1)

#define K_c (1023.0/4.98)
#define M_c (-6.0)


#define K_tot (K_t*K_a*K_c)
#define M_tot (M_t*K_a*K_c + M_a*K_c + M_c)


void EepromRead(uint16_t address, uint16_t length, char *data){
	for(uint16_t a = address ; a < address+length ; a++){
		*data = EEPROM.read(a);
		data++;
	}
}

void EepromWrite(uint16_t address, uint16_t length, char *data){
	for(uint16_t a = address ; a < address+length ; a++){
		EEPROM.write(a, *data);
		data++;
	}
}

void loadChannel(uint8_t ch){
	EepromRead(ch*8+0, 4, (char*)&vOffset[ch]);
	EepromRead(ch*8+4, 4, (char*)&gain[ch]);
}

void saveChannel(uint8_t ch){
	EepromWrite(ch*8+0, 4, (char*)&vOffset[ch]);
	EepromWrite(ch*8+4, 4, (char*)&gain[ch]);
}

uint32_t tomV2(uint32_t v, uint8_t ch){
	return ((v*5>>10)*1000)/(gain[ch]>>16)+vOffset[ch];
}

uint32_t adcTomV(uint32_t adc, uint8_t ch){
	uint32_t v = (adc*5) >> 10; 
	uint32_t mv = v*1000;
	uint32_t gainCorr = (mv/(gain[ch]>>16));
	uint32_t offsetCorr = gainCorr+vOffset[ch];
	return offsetCorr;
}

uint32_t mvToAdc(uint32_t mvo, uint8_t ch){
	int32_t gainCorr = mvo -vOffset[ch];
	if(gainCorr < 0) return 0;
	uint32_t mv = gainCorr*(gain[ch]>>16);
	uint32_t v = mv/1000;
	uint32_t adc = (v/5)<<10;
	return adc;
	//return (((mv-vOffset[ch])*(gain[ch]>>16)))*5;
	//return ((v*5>>10)*1000)/(gain[ch]>>16)+vOffset[ch];
}


float tomV(float x, int ch){
	return 1000.0*(5.0*x/1024.0)/525.0+vOffset[ch];
}

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


void setupAdc(){
	ADCSRA = 1<< ADEN | 1<<ADIE | 1<<ADPS2 | 1 << ADPS1; //Enable ADC, interrupts and prescale adcclk by 64
	ADMUX = 1<<REFS0 | 1<<MUX3 | 1<<MUX2 | 1<<MUX1; //Set REFS to 01 which is the AVCC pin
	sei();
	ADCSRA |= 1<<ADSC; //Start first conversion
}

void setup()
{
	for(int ch = 0 ; ch < 4 ; ch++){
	loadChannel(ch);
	pinMode(heaterPins[ch], OUTPUT);
	digitalWrite(heaterPins[ch], LOW);
	//loadChannel(1);
	//loadChannel(2);
	//loadChannel(3);
	}
	//pinMode(ch1TempPin, INPUT);
	//pinMode(chargePumpPin, OUTPUT);
	//Serial.begin(115200);
	analogReference(DEFAULT);
	//analogWrite(chargePumpPin, 127);
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

	for(int ch = 0; ch < 4 ; ch++){
		ts[ch].temperature_target = 180;
		//ts[ch].adc_value_target = complete(ts[ch].temperature_target);
		ts[ch].adc_value_target = mvToAdc(celciusToMillivolts(((uint32_t)ts[ch].temperature_target)<<16), ch);
		pinMode(heaterPins[ch], OUTPUT);
	}
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
		//Current value is integrated 64 times, so rescale to 16.16
		ts[ch_read].adc_value =  (current_value)<<10; 
		current_value = 0;
		samps = 0;
		ch_read++;
		ch_read = ch_read & 0x3;
		switch(ch_read){
			case 0:
				ADMUX = (ADMUX & 0xF0) | 0; break;
			case 1:
				ADMUX = (ADMUX & 0xF0) | 1; break;
			case 2:
				ADMUX = (ADMUX & 0xF0) | 6; break;
			case 3:
				ADMUX = (ADMUX & 0xF0) | 7; break;
		}
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



enum buttonAction getButtonAction(){
	static uint8_t pressCount = 0;
	static uint8_t lastButtonState = 1;
	uint8_t buttonState = digitalRead(8);
	enum buttonAction ba = NONE;

	if(buttonState == lastButtonState and buttonState == 0){
		Serial.print("pressCount ");Serial.println(pressCount);
		pressCount += 1;
	}

	if(buttonState != lastButtonState and buttonState != 0){
		if(pressCount < 2){ ba = PRESS; Serial.println("SHORT");}
		if(pressCount > 10){ ba = LONG_PRESS; Serial.println("LONG");}
	}

	if(buttonState != lastButtonState) pressCount = 0;
	lastButtonState = buttonState;
	return ba;
}

int32_t getEncoderDelta(){
	static int32_t lastPos = 0;
	int32_t delta = encoder0Pos-lastPos;
	int32_t ad = delta >> 2;
	if(ad)
		lastPos += ad<<2;
	return ad;
}

void cursorLogic( struct menuState *m){
	static uint8_t lastButtonState;
	static uint8_t start;
	int newT;
	uint8_t buttonState = digitalRead(8);
	if(m->channelIsSelected == 0){
		m->cursorChannel = (encoder0Pos >> 2)&0x03;
	} else {
		newT = start + encoder0Pos;
		if(newT < 120){
			newT = 120;
			encoder0Pos = newT-start;
		}if(newT > 400){
			newT = 400;
			encoder0Pos = newT-start;
		}
		ts[m->cursorChannel].temperature_target = newT;
	}
	if(buttonState != lastButtonState){
		if(buttonState != 0){
			if(m->channelIsSelected == 0){
				m->channelIsSelected = 1;
				start = ts[m->cursorChannel].temperature_target;
				encoder0Pos = 0;
			}else{
				//ts[m->cursorChannel].adc_value_target = complete(ts[m->cursorChannel].temperature_target);
				ts[m->cursorChannel].adc_value_target = 3; //mvToAdc(celciusToMillivolts(ts[m->cursorChannel].temperature_target<<16), m->cursorChannel);
				m->channelIsSelected = 0;
				encoder0Pos=m->cursorChannel << 2;
			}
		}
	}
	if(buttonState == lastButtonState and buttonState == 0){
		m->pressCount += 1;
	} else {
		m->pressCount = 0;
	}
	lastButtonState = buttonState;
}


void displayChannel(uint8_t ch, float setPoint, float actual, float power, struct menuState *m)
{ //                      01234567890123456789
	//const char templ[] = "0>383C(383.1C) 30.3W";
	char toPrint2[64];
	if(ch == m->cursorChannel){
		if(m->channelIsSelected == 0)
			sprintf(toPrint2, "%d>%03dC(000.0C) 00.0W", ch, int(setPoint));
		else
			sprintf(toPrint2, "%d=%03dC(000.0C) 00.0W", ch, int(setPoint));
	}
	else
		sprintf(toPrint2, "%d %03dC(000.0C) 00.0W", ch, int(setPoint));
	ftoa(toPrint2+7, actual, 3,1);
	ftoa(toPrint2+15, power, 2,1);
	ftoa(toPrint2+7, actual, 3,1);
	ftoa(toPrint2+15, power, 2,1);
	toPrint2[20]='\0';
	lcd.setCursor(0,ch);
	lcd.print(toPrint2);

}

float regulateChannel(float target, float current){
	return (target - current)*3;
}

void controlChannel(volatile struct tipState *tip){
	int32_t error = (tip->adc_value_target>>12) - (tip->adc_value>>12);
	//Serial.print("Error: "); Serial.print(error); Serial.print(" ");
	if(error < 0)
		error = 0;
	if(error > 85)
		error = 85;
	tip->pwm_value = error*1;
} 

void printControlChannel(volatile struct tipState *tip){
	Serial.print("tadc "); Serial.print(tip->adc_value_target);
	Serial.print(" cadc "); Serial.print(tip->adc_value);
	Serial.print(" pwm "); Serial.print(tip->pwm_value);
	Serial.print(" temp "); Serial.print(ts->temperature);
	Serial.println("");
}

void dprintf(uint8_t line, const char *fmt, ...){
	va_list va;
	va_start(va, fmt);
	char printBuf[21];
	vsnprintf(printBuf, 21, fmt, va);
	lcd.setCursor(0, line);
	lcd.print(printBuf);
	va_end(va);
}

void displayTs(uint8_t ch, bool selected, bool active){
	static char c;
	static uint32_t dispFilt[4] = {0, 0, 0, 0};
	static uint32_t alpha = 0.125*65536;
	static uint32_t one = 65535;
	uint32_t r = 2.1*(1<<8);
	uint32_t v = 12<<8;
	
	uint32_t power = (((uint32_t)60)*(((uint32_t)ts[ch].pwm_value)<<9));
	dispFilt[ch] = (dispFilt[ch]>>8)*((one-alpha)>>8)+(power>>8)*(alpha>>8);
	power = dispFilt[ch]>>16;
	Serial.print("Power: ");Serial.print(power);Serial.print("% ");
	if(selected){
		if(active) c = '=';
		else c ='>';
	} else c = ' ';
	if(ts[ch].temperature < 500)
	dprintf(ch, "%c %d %3uC (%3uC) %3uW         ", c, ch, ts[ch].temperature, ts[ch].temperature_target, power);
	else
	dprintf(ch, "%c %d tip  (%3uC) %3uW         ", c, ch, ts[ch].temperature_target, power);
	//dprintf(ch, "%c %d % 5d(% 3d)           ", c, 3, 4, 5);
}

void mainMenu(enum buttonAction ba, int32_t ed){
	static bool active = false;
	static int8_t selected=0;

	if(ba == PRESS) active = not active;

	if(not active){
		selected = clamp(0, selected+ed, 3);
	}

	if(active){
		ts[selected].temperature_target = clamp(180, ts[selected].temperature_target+ed, 450);
		//Serial.print(" t "); Serial.print(ts[selected].temperature_target);
		//ts[selected].adc_value_target = mvToAdc(celciusToMillivolts(ts[selected].temperature_target<<16), selected);
		ts[selected].adc_value_target = mvToAdc(celciusToMillivolts(((uint32_t) ts[selected].temperature_target)<<16), selected);
	}
	
	for(uint8_t i = 0 ; i < 4 ; i++)
		displayTs(i, selected == i , active); 
}


void serviceMenu(enum buttonAction ba, int32_t ed){
	static int16_t save;
	static int8_t ch=0;
	static bool active = false;
	static int8_t selected=0;
	char v[6];
	static char c;
	char cc;
	uint32_t mv;

	if(ba == PRESS) active = not active;

	if(not active){
		selected = clamp(0, selected+ed, 3);
	}
	if(active){
		if(selected == 0) ch = clamp(0, ch+ed, 3);
		if(selected == 1) vOffset[ch] = vOffset[ch]+(ed<<8);
		if(selected == 2) gain[ch] = gain[ch]+(ed<<16);
		if(selected == 3) save = clamp(-10, save+ed, 10);
	}

	if(active) c = '=';
	else c ='>';
	
	mv = ts[ch].adc_value;
	//Serial.print("Adc_code "); Serial.print(mv) ; Serial.print(" as mv "); Serial.print(adcTomV(mv, ch)/65536.0);
	//Serial.print(" converted back "); Serial.println(mvToAdc(adcTomV(mv, ch), ch));

	dtostrf(tomV2(mv, ch)/65536.0, 2, 3, v);
	cc = selected == 0 ? c : ' ';
	dprintf(0, "%cAdj ch %d %smV         ", cc, (int)ch, v);


	dtostrf(vOffset[ch]/65536.0, 2, 3, v);
	cc = selected == 1 ? c : ' ';
	dprintf(1, "%cOffset  %smV          ", cc, v);
	
	cc = selected == 2 ? c : ' ';
	dtostrf(((float)celciusToMillivolts(millivoltsToCelcius(adcTomV(mv,ch))))/65536.0, 3, 2, v);
	dprintf(2, "%cGain %d         ", cc, gain[ch]>>16);
	//dprintf(2, "%c %sC         ", cc, v);


	cc = selected == 3 ? c : ' ';
	dprintf(3, "%cSave/rst %d 10s/-10r", cc, save);
	if(save >= 10){
		 save = 0;
		 saveChannel(ch); 
	}
	if(save <= -10){
		save = 0;
		vOffset[ch] = 0x10000;
		gain[ch] = ((uint32_t)525)<<16;
	}

}

void menu(){
	static struct menuState ms = {0, 0, 0};
	static uint8_t mode=0;
	enum buttonAction ba;
	uint16_t ch;
	int32_t ed = getEncoderDelta();
	ba = getButtonAction();
	if(mode == 0){/*
		cursorLogic(&ms);
		for(ch = 0 ; ch < 4 ; ch ++){
			displayChannel(ch, ts[ch].temperature_target, tomV2(ts[ch].adc_value, ch)/65536.0/0.016435, ms.pressCount, &ms); 
		}*/
		mainMenu(ba, ed);
		if(ba == LONG_PRESS) mode = 1;
	} else if(mode == 1){
		if(ba == LONG_PRESS) mode = 0;
		serviceMenu(ba, ed);
	}
}

void loop()
{
	uint16_t ch;
	static long rate = 0;
	for(ch = 0 ; ch < 4 ; ch ++){
		ts[ch].temperature = float(ch); //inv_complete(ts[ch].adc_value);
		ts[ch].temperature = millivoltsToCelcius(adcTomV(ts[ch].adc_value, ch))>>16;
		controlChannel(&ts[ch]);
	}

	/*for(ch = 0 ; ch < 4 ; ch ++){
	//displayChannel(ch, int(ts[ch].temperature_target), ts[ch].temperature, (12.0*12.0/2.7)*(ts[ch].pwm_value)/128.0); 
	//displayChannel(ch, int(ts[ch].temperature_target), (float)ts[ch].temperature, (float)tomV((float)ts[ch].adc_value, ch)); 
	displayChannel(ch, ts[ch].temperature_target, ts[ch].temperature, ms.pressCount, &ms); 
	}*/
	menu();	
	delay(10);
	commandLine();
	printControlChannel(&ts[0]);
	//Serial.print("Ch0: "); Serial.print(ts[0].adc_value); 
	//Serial.print(" Enc "); Serial.print (encoder0Pos >> 2, DEC);
	//Serial.print(" Iteration time "); Serial.print(micros()-rate); 
	//Serial.print(" samps: "); Serial.print(sampCount);Serial.println("");
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

