#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <RTClib.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <BH1750.h>

RTC_PCF8523 rtc;
TaskHandle_t TaskBMP280;
TaskHandle_t TaskBH1750;
TaskHandle_t TaskRTC;
TaskHandle_t TaskMQTT; 

AsyncWebServer server(80);

Adafruit_BMP280 bmp;
BH1750 lightMeter;


const char* ssid = "TP-Link_BB96";
const char* password = "Casagatti1";
const char* mqtt_server = "192.168.1.105";

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect(){
  while(!client.connected()){
    Serial.print("Prova connessione MQTT...");
    if(client.connect("ESP32Client")){
      Serial.println("Connesso!");
      //client.subscribe("sensor_data");
    }else{
      Serial.print("Fallito, rc=");
      Serial.print(client.state());
      Serial.println("Ritenta tra 5 secondi");
      delay(5000);
    }
  }
}

void mqttTask(void *pvParameters){
  while(1){
    if(!client.connected()){
      reconnect();
    }
    client.loop();
    vTaskDelay(1000/ portTICK_PERIOD_MS);
  }
}

void readRTCData(void *pvParameters){
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

  void readBH1750Data(void *pvParameters){
    while(1){
  float lux = lightMeter.readLightLevel();
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println("lux");

  vTaskDelay(5000 / portTICK_PERIOD_MS);

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

  
  WiFi.begin(ssid, password );
  while(WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.println("Connessione WiFi in corso...");
  }
  Serial.println("WiFi Connesso");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

  xTaskCreatePinnedToCore(
    readBMP280Data,
    "TaskBMP280",
    10000,
    NULL,
    1,
    &TaskBMP280,
    1);

    xTaskCreatePinnedToCore(
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
    0);

    xTaskCreatePinnedToCore(
    mqttTask,
    "TaskMqtt",
    10000,
    NULL,
    1,
    &TaskMQTT,
    1);

    //Inizializzazione Server Web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
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
}

void loop(){
  if(!client.connected()){
    reconnect();
  }

  float temperatura = bmp.readTemperature();

  String payload = String(temperatura);
  client.publish("temperatura", payload.c_str());
}
