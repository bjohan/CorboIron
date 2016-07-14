#include <LiquidCrystal_I2C.h>
extern "C"
{
#include "uart_command_lib.h"
#include "corbomite.h"
}
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
  lcd.begin(16,2);
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello!11");
}


float readAdc()
{
  int val;
  float valAcc = 0;
  analogWrite(heaterPin, 0);
  delay(1);
  for(int i = 0 ; i < 64 ; i++){
    val = analogRead(ch1TempPin);
    valAcc += val;
  }
  analogWrite(heaterPin, heaterLevel);
  rawAdcValue = val;
  return float(valAcc)/64.0;
}


void setHeatSafe(int level)
{
digitalWrite(heaterPin, 0);
 /* heaterLevel = level;
  if(rawAdcValue < 700){
    if(level > 0) 
        digitalWrite(heaterPin, 1);
    else
        digitalWrite(heaterPin, 0);
    //analogWrite(heaterPin, heaterLevel);
  } else {
    digitalWrite(heaterPin, 0);
    //analogWrite(heaterPin, 0);
  }
*/
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

/*void regulateModel(float target, float dT)
{
  static long long int nextTime = 0;
  static float energyFilter = 0;
  const float energyAlpha = 0.01;
  float hlev = 20;
  float temp = toCelcius(readAdc());
  float diff = target-temp;
  float totalThermalEnergyToAdd = diff/MIN_MASS;
  float thermalEnergyToAdd = totalThermalEnergyToAdd - energyFilter;
  
  if(thermalEnergyToAdd < 0 )
    thermalEnergyToAdd = 0;
  
  float extraPower = 0.1*thermalEnergyToAdd/dT;
  

    
  energyFilter = energyFilter*(1.0-energyAlpha)+thermalEnergyToAdd;
  
  float radPower = (target-AMBIENT)/RTH_NOM_AIR;
  
  hlev = powerToDuty(radPower+extraPower);
  //hlev = powerToDuty(5.0);
  
  while(millis() < nextTime);
//    Serial.println("looping");
  
  setHeatSafe(hlev);
  nextTime = millis()+1000*dT;
  printValue("target: ", target);
  printValue(" prad ", radPower);
  printValue(" temperature: ", temp);
  printValue(" extraPower: ", extraPower);
  printValue(" filtered energy: ", energyFilter);
  printValue(" tot therm e: ", totalThermalEnergyToAdd);
  printValue(" Pow: ", (hlev/255)*12*12/3.6);
  Serial.println("");
}*/

void regulatePidWithCompensation(float target, float dT)
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
  float error = target-temp/*+setpointIncrease*/;
  float tFlow;
  
  tFlow = (lastPower-5*temp/(250));
  
  float pTerm = error*sP/dT;
  float iTerm = integral*sI/dT;
  float dTerm = (error -lastError)*sD/dT;
/*  float pTerm = error*1.5;
  float iTerm = integral*0.3;
  float dTerm = (error -lastError)*0.7/dT;*/
  
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
  
  int hlev = powerToDuty(controlPower/*+setpointIncrease*/);
  
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
  /*printValue("target: ", target);
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
  Serial.println("");*/
}

/*void regulatePid(float target, float dT)
{

  static long long int nextTime = 0;
  static float lastError = 0;
  static float integral = 0;
  static float lastPower = 0;
  
  float temp = toCelcius(readAdc());
  float error = target-temp;
  float tFlow;
  
  tFlow = (lastPower-5*temp/(250));
  
  float pTerm = error*0.2;
  float iTerm = integral*0.08;
  float dTerm = -(error -lastError)*0.01/dT;
  
  if(iTerm < 40.0)
    integral += error*dT;
  if (error < -15)
    integral = 0;
  if (error > 5 and integral < 0)
     integral = 0;
  //integral = integral > 3000 ? 3000 : integral;
  lastError = error;  
  
  float control = pTerm+iTerm+dTerm;
  int hlev = powerToDuty(control);
  hlev = hlev > 255 ? 255 : hlev;
  hlev = hlev < 0 ? 0 : hlev;

  
  while(millis() < nextTime);
//    Serial.println("looping");
  
  setHeatSafe(hlev);
  nextTime = millis()+1000*dT;
  lastPower = (float(hlev)/255.0)*12.0*12.0/3.6;
  printValue("target: ", target);
  printValue(" temperature: ", temp);
  printValue(" tFlow: ", tFlow);
  printValue(" power: ", control);
  printValue(" pwm: ", hlev);
  printValue(" p: ", pTerm);
  printValue(" i: ", iTerm);
  printValue(" d: ", dTerm);
  printValue(" Pow: ", lastPower);
  Serial.println("");
}

void regulate(int target)
{
  static float integral = 0;
  static float lastVal = 0;
  float alpha = 1.0/20.0;
  
  float p = 0.5;
  float i = 0.01;
  float d = 0;
  float pterm;
  float iterm;
  float dterm;
  
  float val = readAdc();
  float err = target - val;
  integral += err; //+ integral*(1.0-alpha);
  float delta = val - lastVal;
  
  pterm = err*p;
  iterm = integral*i;
  dterm = delta*d;
  float hlev = 0;
  hlev = pterm+iterm+dterm;
  hlev = hlev > 255 ? 255 : hlev;
  
  
  printValue("Value: ", val);
  printValue(" error: ", err);
  printValue(" integral: ", integral);
  printValue(" delta: ", delta);
  
  printValue(" P: ", pterm);
  printValue(" I: ", iterm);
  printValue(" D: ", dterm);
  printValue(" S: ", hlev);
  printValue(" Pow: ", (hlev/255)*12*12/3.6);
  Serial.println("");
  if(hlev > 0)
    digitalWrite(heatLed, HIGH);
  else
      digitalWrite(heatLed, LOW);
  setHeatSafe(hlev);
  lastVal = val;
}*/

void loop()
{
/*  int hlev;
  int val = readAdc();
  if(val < 600){
    
    hlev = 200;
  } else {
     digitalWrite(heatLed, LOW);
     hlev = 0;
  }
  setHeatSafe(hlev);*/
  regulatePidWithCompensation(setPoint,0.05);
 /* Serial.print("addr1 ");
  Serial.print((uint32_t)&sensorTemperature);
  Serial.print("addr2 ");
  Serial.print((uint32_t)&initcmd);
  Serial.print("addr3 ");
  Serial.print((uint32_t)&last);
  Serial.print("addr4 ");
  Serial.print((uint32_t)&entries);*/
  commandLine();
  //delay(200);
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
 
