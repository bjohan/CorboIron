#include <LiquidCrystal_I2C.h>
extern "C"
{
#include "uart_command_lib.h"
#include "corbomite.h"
}
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
ANA_IN("temperature", "C", "0", "450", 0, 450*64, temperature);
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
void setup()
{
        pinMode(ch1TempPin, INPUT);
        Serial.begin(115200);
        analogReference(INTERNAL);
        analogWrite(chargePumpPin, 127);
        pinMode(heatLed, OUTPUT);
        pinMode(ch1TempPin, INPUT);
        pinMode(heaterPin, OUTPUT);
        digitalWrite(heaterPin, 0);
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


float readAdc(int ch)
{
  int val;
  float valAcc = 0;
  //analogWrite(heaterPin, 0);
  delay(1);
  for(int i = 0 ; i < 64 ; i++){
    val = analogRead(tempPins[ch]);
    valAcc += val;
  }
  //analogWrite(heaterPin, heaterLevel);
  rawAdcValue = val;
  return float(valAcc)/64.0;
}


void setHeat(int ch, uint8_t en)
{
        digitalWrite(heaterPins[ch], en);
}

void setHeatSafe(int level)
{
        digitalWrite(heaterPin, 0);
}

void printValue(char *name, float value)
{
  Serial.print(name);
  if (value > 0)
   Serial.print("+");
  Serial.print(value);
}

float toCelcius(float adcValue)
{
  return (adcValue/1.6-137.5)*200.0/170.0;
}

int powerToDuty(float power)
{
  int duty = int(255*(power/(12*12/3.6))+0.5);
  return duty > 255 ? 255 : duty;
}

#define MAX_POWER (12.0*12.0/3.6)
#define MIN_MASS 10.0 //joules per celcius
#define AMBIENT  23.0
#define RTH_NOM_AIR ((270.0-AMBIENT)/5.0) //Kelvin per watt

/*void regulatePidWithCompensation(float target, float dT)
{

  static long long int nextTime = 0;
  static float lastError = 0;
  static float integral = 0;
  static float lastPower = 0;
  static float setpointIncrease = 0;
  //int acc = 0;
  int i; 
  if (millis() < nextTime)
    return;

  float temp = 0;
  //for(i = 0 ; i < 4 ; i++)
  //  acc+=readAdc();
  temp=toCelcius(readAdc());
  printValue("Raw", temp);
  //temp=toCelcius(float(acc)/4.0);
  float error = target-temp;
  float tFlow;
  
  tFlow = (lastPower-5*temp/(250));
  
  float pTerm = error*sP/dT;
  float iTerm = integral*sI/dT;
  float dTerm = (error -lastError)*sD/dT;
  
  if(iTerm < 40.0)
    integral += error*dT;
  if (error < -15)
    integral = 0;
  if (error > 10 and integral < 0)
     integral = 0;
  //integral = integral > 3000 ? 3000 : integral;
  lastError = error;  
  
  float controlPower = pTerm+iTerm+dTerm;
  setpointIncrease = controlPower*2;
  setpointIncrease = setpointIncrease > 0 ? setpointIncrease : 0;
  
  int hlev = powerToDuty(controlPower);
  
  hlev = hlev > 255 ? 255 : hlev;
  hlev = hlev < 0 ? 0 : hlev;
  
  
//    Serial.println("looping");
  
  setHeatSafe(hlev);
  nextTime = millis()+1000*dT;
  lastPower = (float(hlev)/255.0)*12.0*12.0/3.6;

  transmitAnalogIn(&powerWidget, (int) hlev);
  transmitAnalogIn(&pPowerWidget, (int) (pTerm*100.0));
  transmitAnalogIn(&iPowerWidget, (int) (iTerm*100.0));
  transmitAnalogIn(&dPowerWidget, (int) (dTerm*100.0));
  transmitAnalogIn(&temperature, (int) (temp*64.0));
  printValue("target: ", target);
  printValue(" value: ", target+setpointIncrease);
  printValue(" spi:", setpointIncrease);
  printValue(" temperature: ", temp);
  printValue(" tFlow: ", tFlow);
  printValue(" power: ", controlPower);
  printValue(" pwm: ", hlev);
  printValue(" p: ", pTerm);
  printValue(" i: ", iTerm);
  printValue(" d: ", dTerm);
  printValue(" Pow: ", lastPower);
  Serial.println("");
}*/

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

  lcd.setCursor(0,ch);
  lcd.print(toPrint2);

}

float regulateChannel(uint8_t ch, float setPoint, float temperature)
{
        float error = setPoint-temperature;
        if (error > 0.0){
                error = error * 0.01;
                return error > 0.2 ? 0.2 : error;
        }
        return 0;
}
 

#define MAX_POWER ((12.0f*12.0f/2.0f)*(200.0f/250.0f))


void loop()
{
        long t0;
        float target = 200;
        float temperatures[4];
        float powerFactors[4];
        int milliseconds[4]; 
        long time;
        uint8_t ch;
        t0 = millis();
        for(ch = 0 ; ch < 4 ; ch ++){
                temperatures[ch] = toCelcius(readAdc(ch));
                powerFactors[ch] = regulateChannel(ch, target, temperatures[ch]);
                milliseconds[ch] = int(powerFactors[ch]*100.0f); 
        }

        for(ch = 0 ; ch < 4 ; ch ++){
                displayChannel(ch, int(target), temperatures[ch], powerFactors[ch]*MAX_POWER); 
        }
        while(millis()-t0 < 50);
        t0 = millis();
        time = millis() -t0;
        while(time <= 100){
                for(ch = 0 ; ch < 4 ; ch ++){
                        if(time < milliseconds[ch])
                                setHeat(ch, 1);
                        else
                                setHeat(ch, 0);
                }
                time = millis() -t0;

        }

        //heaterLevel = 250*powerFactors[0];
        printValue("Time: ", millis()-t0);
        Serial.println("");

        commandLine();
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
 
