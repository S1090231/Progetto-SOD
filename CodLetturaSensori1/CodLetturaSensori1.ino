#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

#define BMP_SDA 21
#define BMP_SCL 22


Adafruit_BMP280 bmp;
BH1750 lightmeter;

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
  
  delay(1000);}

  