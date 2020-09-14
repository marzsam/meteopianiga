#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

#define BT_TX_PIN 12
#define BT_RX_PIN 11
SoftwareSerial bt =  SoftwareSerial(BT_RX_PIN, BT_TX_PIN);

Adafruit_BME280 bme;

int wind_direction_pin = A0;

int solar_pin = A1;
float solar_k = 1.0;

const int sharpLEDPin = 7;
const int sharpVoPin = A2; 
static float Voc = 0.6;
const float K = 0.5;

int wind_int_count;
long last_wind_int;

void setup() {
  Serial.begin(9600);

  pinMode(BT_RX_PIN, INPUT);
  pinMode(BT_TX_PIN, OUTPUT);
  bt.begin(9600);
  
  bme.begin(); 

  pinMode(sharpLEDPin, OUTPUT);

  wind_int_count = 0;
  last_wind_int = millis();
  attachInterrupt(digitalPinToInterrupt(2), wind_int, RISING);

  wdt_enable(WDTO_8S);
}

void loop() {
  
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  Serial.println("Temp: "+(String)temperature+" degC Hum: "+(String)humidity+"% Press: "+(String)pressure);

  float wind_direction = get_wind_direction();
  Serial.println("Wind dir: "+(String)wind_direction+" deg");

  float solar_radiation = 0.0;
  for(int i=0; i<100; i++){
    solar_radiation = solar_radiation + analogRead(solar_pin)*(5.0/1024.0)*solar_k;
    }
  solar_radiation = solar_radiation/100.0;
  Serial.println("Solar radiation: "+(String)solar_radiation+" V");

  float dust_density = get_dust_density();
  Serial.println("Dust density: "+(String)dust_density+" mg/m^3");

  float wind_speed = wind_int_count*(2.4/5.0);
  wind_int_count = 0;
  Serial.println("Wind speed: "+(String)wind_speed+" km/h");

  bt.println((String)temperature+";"+(String)humidity+";"+(String)pressure+";"+(String)wind_direction+";"+(String)solar_radiation+";"+(String)dust_density+";"+(String)wind_speed);

  delay(4000);

  wdt_reset();
}

float get_wind_direction(){
  float wind_direction_volt = analogRead(wind_direction_pin)*(5.0/1024.0);
  float direction_volt [16] = {3.84,1.98,2.25,0.41,0.45,0.32,0.90,0.62,1.40,1.19,3.08,2.93,4.62,4.04,4.33,3.43}; 
  for(int i=0; i<16; i++){
    direction_volt[i] = abs(direction_volt[i] - wind_direction_volt);
    }
  float min_direction_volt = direction_volt[0];
  int min_index = 0;
  for(int i=1; i<16; i++){
    if( direction_volt[i] < min_direction_volt){
        min_direction_volt = direction_volt[i];
        min_index = i;
      }
    }
  return min_index * 22.5;
}

float get_dust_density(){
  long VoRaw = 0;
  for(int i=0; i<100; i++){
    digitalWrite(sharpLEDPin, LOW);
    delayMicroseconds(280);
    VoRaw = VoRaw + analogRead(sharpVoPin);
    digitalWrite(sharpLEDPin, HIGH);
    delayMicroseconds(9620);
  }
  float Vo = VoRaw/100.0;
  Vo = Vo / 1024.0 * 5.0;
  float dV = Vo - Voc;
  if ( dV < 0 ) {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;
  return dustDensity;
}

void wind_int(){
  if( (millis()-last_wind_int)>=30 ){ //max speed 80 km/h
    wind_int_count++;
    last_wind_int = millis();
  }
}
