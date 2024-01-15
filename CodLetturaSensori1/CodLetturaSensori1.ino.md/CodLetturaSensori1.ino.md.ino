#include <Wire.h> 
//#include <Arduino_FreeRTOS.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include "RTClib.h"

#define BMP_SDA 21
#define BMP_SCL 22


Adafruit_BMP280 bmp;
BH1750 lightmeter;
RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup(){
  Serial.begin(9600);
  

 //Inizializza i sensori
  if(!bmp.begin()){
    Serial.println("Errore BMP280!");
    while(1);
  }
  
  if(!lightmeter.begin()){
    Serial.println("Errore BH1750!");
    while(1);
  }
  lightmeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void loop(){
  Serial.print("Temperatura = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressione = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  
  uint16_t lux = lightmeter.readLightLevel();
  Serial.print("Luminosità: ");
  Serial.print(lux);
  Serial.println("lux");

  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

  
  delay(5000);}

  