#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <BH1750.h>

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BMP280 bmp;
BH1750 lightMeter;

TaskHandle_t TaskBMP280;
TaskHandle_t TaskBH1750;

void readBMP280Data(){
  float temperature, pressure;
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure () / 100.0F;
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Pressure: ");
  Serial.print(pressure);
  Serial.println("hPa");

  String payload = "Temperature: " + String(temperature) + "°C, Pressure: " + String(pressure) + "hPa";
  client.publish("sensor_data", payload.c_str());
}

void readBH1750Data(){
  float lux = lightMeter.readLightLevel();
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println("lux");

  String payload = "Light intensity: " +String(lux) + "lux";
  client.publish("sensor_data", payload.c_str());
  }

  void TaskBMP280code(void *pvParameters){
    (void) pvParameters;
    for(;;){
      readBMP280Data();
      delay(5000);
    }
  }

  void TaskBH1750code(void *pvParameters){
    (void) pvParameters;
    for(;;){
      readBH1750Data();
      delay(5000);
    }
  }

void reconnect(){
  while(!client.connected()){
    Serial.print("Prova connessione MQTT...");
    if(client.connect("ESP32Client")){
      Serial.println("Connesso!");
      client.subscribe("sensor_data");
    }else{
      Serial.print("Fallito, rc=");
      Serial.print(client.state());
      Serial.println("Ritenta tra 5 secondi");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  if(!bmp.begin()){
    Serial.println("Errore inizializzazione BMP280");
    while(1);
  }

  if(!lightMeter.begin()){
    Serial.println("Errore inizializzazione BH1750");
    while(1);
  }
  
  WiFi.begin("TP-Link_BB96", "Casagatti1" );
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione WiFi in corso...");
  }
  Serial.println("WiFi Connesso");

  client.setServer("192.168.1.105", 1883);

  xTaskCreatePinnedToCore(
    TaskBMP280code,
    "TaskBMP280",
    10000,
    NULL,
    1,
    &TaskBMP280,
    0);

    xTaskCreatePinnedToCore(
    TaskBH1750code,
    "TaskBH1750",
    10000,
    NULL,
    1,
    &TaskBH1750,
    0);
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();

}
