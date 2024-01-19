#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>
#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <BH1750.h>

RTC_PCF8523 rtc;
//TaskHandle_t TaskBMP280;
//TaskHandle_t TaskBH1750;
TaskHandle_t TaskSensorData;
TaskHandle_t TaskMQTT; 
SemaphoreHandle_t dataMutex;

//Inizializzazione del server Web sulla porta 80
//AsyncWebServer server(80);

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

    xSemaphoreTake(dataMutex, portMAX_DELAY);

    luminosity = currentLuminosity;
    temperature = currentTemperature;
    pressure = currentPressure;
    timestamp = currentTimestamp;

    xSemaphoreGive(dataMutex);

    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

//Funzione per riconnettersi al broker MQTT in caso di disconnessione
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


//Task per la gestione della connessione e comunicazione MQTT
/*void mqttTask(void *pvParameters){
  while(1){
    if(!client.connected()){
      reconnect();
    }
    client.loop();
    vTaskDelay(1000/ portTICK_PERIOD_MS);
  }
}*/

//Task per la lettura del timestamp dal modulo RTC
/*void readRTCData(void *pvParameters){
    TickType_t lastWakeTime = xTaskGetTickCount();

    while(1){
      DateTime now = rtc.now();
      Serial.print("Timestamp RTC: ");
      Serial.print(now.year(), DEC);
      Serial.print("-");
      Serial.print(now.month(), DEC);
      Serial.print("-");
       Serial.print(now.day(), DEC);
       Serial.print("-");

    
    Serial.print("- ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    vTaskDelayUntil(&lastWakeTime, 1000 / portTICK_PERIOD_MS);
    }
  }

//Task per la lettura dei dati dal sensore BMP280
  void readBMP280Data(void *pvParameters){
    while(1){
  Serial.print("Temperature: ");
  Serial.print(bmp.readTemperature());
  Serial.print("°C, Pressure: ");
  Serial.print(bmp.readPressure() / 100.0F);
  Serial.println("hPa");
  
   vTaskDelay(5000 / portTICK_PERIOD_MS);
}
  }

//Task per la lettura dei dati dal sensore BH1750
  void readBH1750Data(void *pvParameters){
    while(1){
  float lux = lightMeter.readLightLevel();
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println("lux");

  vTaskDelay(5000 / portTICK_PERIOD_MS);

  }
  }*/


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

    /*xTaskCreatePinnedToCore(
    readBH1750Data,
    "TaskBH1750",
    10000,
    NULL,
    1,
    &TaskBH1750,
    1);

    xTaskCreatePinnedToCore(
    readRTCData,
    "TaskRTC",
    10000,
    NULL,
    1,
    &TaskRTC,
    0);*/

    xTaskCreatePinnedToCore(
    sendMQTTData,
    "TaskMQTT",
    10000,
    NULL,
    1,
    &TaskMQTT,
    1); }

    //Inizializzazione Server Web
    /*server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      DateTime now = rtc.now();
          String html = "<html><body>";
          html += "<h1>Sensor data</h1>";
          html += "<p>Timestamp RTC: " + String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + 
                  " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + "</p>";
          html += "<p> Temperatura BMP280: " + String(bmp.readTemperature()) + "&deg;C </p>";
          html += "<p> Pressione BMPP280 : " + String(bmp.readPressure()/100.0F) + "hPa </p>";
          html += "<p> Luminosità BH1750: " + String(lightMeter.readLightLevel()) + "lux </p>";
          html += "</body></html>"; 
          request->send(200, "text/html", html);
    });
    server.begin();
}*/

void loop(){
  /*if(!client.connected()){
    reconnect();
  }

  float temperatura = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  uint16_t luminosita = lightMeter.readLightLevel();
  DateTime now = rtc.now();
  String timestamp = String(now.year()) + "-" +
                     String(now.month()) + "-" +
                     String(now.day()) + " " + 
                     String(now.hour()) + ":" +
                     String(now.minute()) + ":" +
                     String(now.second()); 

  String payloadTemperatura = "Temperatura attuale: " +String(temperatura) + "°C";
  client.publish("temperatura", payloadTemperatura.c_str());


  String payloadPressure = "Pressure attuale: " +String(pressure) + "hPa";
  client.publish("pressure", payloadPressure.c_str());

  String payloadLuminosita = "Luminosità attuale: " +String(luminosita) + "lux";
  client.publish("luminosita", payloadLuminosita.c_str());

  client.publish("timestamp", timestamp.c_str());

  delay(5000);*/
}
