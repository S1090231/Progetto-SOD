#include <Wire.h> 
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include "RTClib.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define BMP_SDA 21
#define BMP_SCL 22

//Credenziali Wi-fi
const char *ssid = "TP-Link_BB96";
const char *password = "Casagatti1";
//Credenziali MQTT
const int mqtt_port = 1883;
const char *mqtt_server = "192.168.1.105";
const char *mqtt_user = "mqtt_user";
const char *mqtt_password = "mqtt1234";
const char *mqtt_topic = "sensor_data";

WiFiClient espClient; 
PubSubClient client(espClient);

void setupWiFi(){
  Serial.begin(9600);
  delay(10);
  
WiFi.begin(ssid,password);
Serial.print("Connessione in corso ");

while(WiFi.status() != WL_CONNECTED){
  delay(500);
  Serial.println(".");
}
Serial.println("Connesso al Wi-Fi");
}

Adafruit_BMP280 bmp;

void bmpTask(void *pvParameters){
  (void)pvParameters;

  for(;;){
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure()/100.0F;
    Serial.print("Temperatura = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressione = ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println(" hPa");

  //Invia dati MQTT
char tempStr[10];
char pressStr[10];
dtostrf(temperature, 4,2,tempStr);
dtostrf(pressure, 6,2,pressStr);

client.publish("temperature", tempStr);
client.publish("pressure", pressStr);

  vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

BH1750 lightmeter;

void bh1750Task(void *pvParameters){
  (void)pvParameters;

  for(;;){
    uint16_t lux = lightmeter.readLightLevel();
  Serial.print("Luminosità: ");
  Serial.print(lux);
  Serial.println("lux");

//Invia dati MQTT
char luxStr[10];
itoa(lux, luxStr, 10);

client.publish("lux", luxStr);

vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

RTC_PCF8523 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void rtcTask(void *pvParameters){
  (void)pvParameters;

  for(;;){
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

    //Invia dati MQTT
    char dateTimeStr[20];
    snprintf(dateTimeStr, sizeof(dateTimeStr), "%04d-%02d-%02d %o2d:%02d:%02d",
             now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    client.publish("datetime", dateTimeStr);         

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void mqttTask(void *pvParameters){
  (void)pvParameters;

  for(;;){
    //Connessione al broker MQTT
    if(!client.connected()){
      Serial.println("Connessione al broker MQTT...");
      if(client.connect("ESP32Client", mqtt_server, mqtt_password)){
        Serial.println("Connesso al broker MQTT!");
      }else{
        Serial.println("Connessione al broker MQTT fallita. Riprova tra qualche secondo...");
        vTaskDelay(pdMS_TO_TICKS(5000));
        continue;
      }
    }
    //Mantieni la connessione MQTT
    client.loop();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}



void setup(){
  Serial.begin(9600);
  delay(1000);
  

 //Inizializza i sensori
  if(!bmp.begin()){
    Serial.println("Errore BMP280!");
    while(1);
  }
  xTaskCreatePinnedToCore(bmpTask, "BMP Task", 4000, NULL, 1, NULL, 0);
  
  if(!lightmeter.begin()){
    Serial.println("Errore BH1750!");
    while(1);
  }
  lightmeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
  xTaskCreatePinnedToCore(bh1750Task, "BH1750 Task", 4000, NULL, 1, NULL, 0);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  xTaskCreatePinnedToCore(rtcTask, "RTC Task", 4000, NULL, 1, NULL, 0);

//Inizializza il client MQTT
client.setServer(mqtt_server,1883);
  /*setup_wifi();
  client.setServer(mqtt_server, 1883);
 // client.setCallback(callback);*/
}



void loop(){
  //Serial.print("Temperatura = ");
  //Serial.print(bmp.readTemperature());
  //Serial.println(" °C");

  //Serial.print("Pressione = ");
  //Serial.print(bmp.readPressure() / 100.0F);
  //Serial.println(" hPa");

  
  //uint16_t lux = lightmeter.readLightLevel();
  //Serial.print("Luminosità: ");
  //Serial.print(lux);
  //Serial.println("lux");

  /*DateTime now = rtc.now();

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

  
  delay(5000);*/}

  