/*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*   
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//tx --> pin 5 | rx --> pin 6
SoftwareSerial bt (5,6);

String dataString;

Adafruit_BME280 bme;

int rainCount;
long lastRainInt;

long lastWindInt;
float windTimeGst;
int windCount;

int windDirPin = A0;
int windDirCount[16];   //array structure: [N, NNE, NE, ... , NNW]
int windDirGst;

int radPin = A1;
float radCal = 1;

int battPin = A2;
float battCal = 1;

int pinLed = 8;

void setup() {
  bt.begin(9600);

  bme.begin();

  pinMode(8, OUTPUT);
  
  rainCount = 0;
  lastRainInt = millis();

  lastWindInt = millis();
  windTimeGst = 0.0;
  windCount = 0;

  for(int i=0; i<16; i++){
    windDirCount[i] = 0;
    }
  windDirGst = 0;
  
  attachInterrupt(digitalPinToInterrupt(2), rainInt, RISING);
  attachInterrupt(digitalPinToInterrupt(3), windInt, RISING);
  }

void loop() {

  for(int i=0; i<60; i++){
    digitalWrite(pinLed, HIGH);
    delay(250);
    digitalWrite(pinLed, LOW);
    delay(750);
    }

  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure()/100.0;
  
  float rain = rainCount*0.2794;
  rainCount = 0;

  float windAvg = 0.0;
  float windGst = 0.0;
  float dirAvg = getWindDirection();
  float dirGst = getWindDirection();
  
  if(windCount > 0){
    float windAvg = 2.4011*(windCount/60.0);
    float windGst = 2.4011/(windTimeGst);
    
    int maxDir = windDirCount[0];
    int maxDirIndex = 0;
    for(int i=0; i<16; i++){
      if(windDirCount[i]>maxDir){
        maxDir = windDirCount[i];
        maxDirIndex = i;
        }
      }
      
    dirAvg = maxDirIndex*22.5;
    dirGst = windDirGst*22.5;
    }
    
  windTimeGst = 0.0;
  windCount = 0;
  for(int i=0; i<16; i++){
    windDirCount[i] = 0;
    }
  windDirGst = 0;

  float rad = analogRead(radPin)*radCal;

  float batt = analogRead(battPin)*battCal;

  dataString = "";

  dataString = dataString + String(temp);
  dataString = dataString + ";";
  
  dataString = dataString + String(hum);
  dataString = dataString + ";";
  
  dataString = dataString + String(pres);
  dataString = dataString + ";";

  dataString = dataString + String(rain);
  dataString = dataString + ";";

  dataString = dataString + String(windAvg);
  dataString = dataString + ";";

  dataString = dataString + String(dirAvg);
  dataString = dataString + ";";

  dataString = dataString + String(windGst);
  dataString = dataString + ";";

  dataString = dataString + String(dirGst);
  dataString = dataString + ";";

  dataString = dataString + String(rad);
  dataString = dataString + ";";

  dataString = dataString + String(batt);
  dataString = dataString + ";";
  
  bt.print(dataString);
  bt.print("\n");
  }

void rainInt(){
  if( (millis()-lastRainInt)>1000 ){
    rainCount++;
    lastRainInt = millis();
    }
  }

void windInt(){
  float deltaSec = (millis()-lastWindInt)/1000.0;
  if(deltaSec > 0.030){
    int dir = getWindDirection();
    windCount++;
    windDirCount[dir]++;
    if((windTimeGst < 0.030)||(windTimeGst > deltaSec)){
      windTimeGst = deltaSec;
      windDirGst = dir;
      }
    lastWindInt = millis();
    }
  }

int getWindDirection(){

  float distance[16];
  float voltage = analogRead(windDirPin)*(5.0/4096.0);
  
  distance[0] = abs(voltage-3.84);
  distance[1] = abs(voltage-1.98);
  distance[2] = abs(voltage-2.25);
  distance[3] = abs(voltage-0.41);
  distance[4] = abs(voltage-0.45);
  distance[5] = abs(voltage-0.32);
  distance[6] = abs(voltage-0.90);
  distance[7] = abs(voltage-0.62);
  distance[8] = abs(voltage-1.40);
  distance[9] = abs(voltage-1.19);
  distance[10] = abs(voltage-3.08);
  distance[11] = abs(voltage-2.93);
  distance[12] = abs(voltage-4.62);
  distance[13] = abs(voltage-4.04);
  distance[14] = abs(voltage-4.78);
  distance[15] = abs(voltage-3.43);

  int minIndex = 0;
  int minValue = distance[0];
  for(int i=1; i<16; i++){
    if(distance[i] < minValue){
      minValue = distance[i];
      minIndex = i;
      }
     }

  return minIndex;
  
  }
