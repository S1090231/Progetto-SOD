#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BH1750.h>

RTC_PCF8523 rtc;
TaskHandle_t TaskSensorData;
TaskHandle_t TaskMQTT; 
SemaphoreHandle_t dataMutex;


Adafruit_BMP280 bmp;
BH1750 lightMeter;


const char* ssid = "TP-Link_BB96";
const char* password = "Casagatti1";
const char* mqtt_server = "192.168.1.105"; //Indirizzo IP del server MQTT
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

float luminosity, temperature, pressure;
String timestamp =" ";

//task per la lettura dei dati dai sensori 
void readSensorData(void *pvParameters){
  while(1){
    float currentLuminosity = lightMeter.readLightLevel();
    float currentTemperature = bmp.readTemperature();
    float currentPressure = bmp.readPressure() / 100.0F;

    DateTime now = rtc.now();
    String currentTimestamp = String(now.year()) + "-" +
                              String(now.month()) + "-" + 
                              String(now.day()) + "" +
                              String(now.hour()) + ":" +
                              String(now.minute()) + ":" +
                              String(now.second());
//richiesta di accesso al semaforo
    xSemaphoreTake(dataMutex, portMAX_DELAY);

    luminosity = currentLuminosity;
    temperature = currentTemperature;
    pressure = currentPressure;
    timestamp = currentTimestamp;

//rilascia il semaforo acquisito precedentemente
    xSemaphoreGive(dataMutex);

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

//Funzione per riconnettersi al broker MQTT in caso di disconnessione e per inviare dati
void sendMQTTData(void *pvParameters){
  while(1){
    if(!client.connected()){
      if(client.connect("ESP32Client")){
      Serial.println("Connesso al broker MQTT!");
    }else{
      Serial.println("Connessione al broker MQTT fallita");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }
  }

  xSemaphoreTake(dataMutex, portMAX_DELAY);

  client.publish("luminosity", String(luminosity).c_str());
  client.publish("temperature", String(temperature).c_str());
  client.publish("pressure", String(pressure).c_str());
  client.publish("timestamp", timestamp.c_str());

  xSemaphoreGive(dataMutex);

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

 void setup() {
  Serial.begin(115200);
  Wire.begin();

  if(!bmp.begin()){
    Serial.println("Errore inizializzazione BMP280");
    while(1);
  }

  if(!lightMeter.begin()){
    Serial.println("Errore inizializzazione BH1750");
    while(1);
  }

  if(!rtc.begin()){
    Serial.println("Impossibile trovare il modulo RTC");
    while(1);
  }
  
//crea un semaforo di mutua esclusione
  dataMutex = xSemaphoreCreateMutex();

  //Connessione alla WiFi
  WiFi.begin(ssid, password );
  while(WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.println("Connessione WiFi in corso...");
  }
  Serial.println("WiFi Connesso");
  Serial.println(WiFi.localIP());

//Configurazione del client MQTT
  client.setServer(mqtt_server, mqtt_port);

  xTaskCreatePinnedToCore(
    readSensorData,
    "TaskSensorData",
    10000,
    NULL,
    1,
    &TaskSensorData,
    0); //core 0

    

    xTaskCreatePinnedToCore(
    sendMQTTData,
    "TaskMQTT",
    10000,
    NULL,
    1,
    &TaskMQTT,
    1); }

    

void loop(){
  delay(5000);
}
