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
RTC_PCF8523 rtc;
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


TaskHandle_t TaskBMP280;
TaskHandle_t TaskBH1750;
TaskHandle_t TaskRTC;

float bmpTemperature, bmpPressure, bh1750Lux;
//String timestamp;

void readBMP280Data(){
  float temperature, pressure;
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure () / 100.0F;
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Pressure: ");
  Serial.print(pressure);
  Serial.println("hPa");
  //timestamp = getTimestamp();

  String payload = " Temperature: " + String(temperature) + "°C, Pressure: " + String(pressure) + "hPa";
  client.publish("sensor_data", payload.c_str());
}

void readBH1750Data(){
  float lux = lightMeter.readLightLevel();
  Serial.print("Light intensity: ");
  Serial.print(lux);
  Serial.println("lux");
  //timestamp = getTimestamp();

  String payload = " Light intensity: " +String(lux) + "lux";
  client.publish("sensor_data", payload.c_str());
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

    //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    vTaskDelayUntil(&lastWakeTime, 1000 / portTICK_PERIOD_MS);
    }}


  //Funzione per ottenere il timestamp dal modulo RTC
  /*String getTimestamp(){
    DateTime now = rtc.now();
    String timestamp = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day()) + " " +
                       String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
    return timestamp;                   
  }*/

  //Funzione per gestire la richiesta HTTP
  void handleRoot(AsyncWebServerRequest *request){
    String html = "<html><body";
    html += "<h1>Sensor Data</h1>";
    html += "<p>BMP280 Temperature: " +String(bmpTemperature)+ "°C </p>";
    html += "<p>BMP280 Pressure: " +String(bmpPressure)+ "hPa </p>";
    html += "<p>BH1750 Light Intensity: " +String(bh1750Lux)+ "lux </p>";
    html += "</html></body>";

    request->send(200, "text/html", html);
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

  if(!rtc.begin()){
    Serial.println("Impossibile trovare il modulo RTC");
    while(1);
  }

  
  WiFi.begin("TP-Link_BB96", "Casagatti1" );
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione WiFi in corso...");
  }
  Serial.println("WiFi Connesso");
  Serial.println(WiFi.localIP());



  client.setServer("192.168.1.105", 1883);

  xTaskCreatePinnedToCore(
    TaskBMP280code,
    "TaskBMP280",
    20000,
    NULL,
    1,
    &TaskBMP280,
    0);

    xTaskCreatePinnedToCore(
    TaskBH1750code,
    "TaskBH1750",
    20000,
    NULL,
    1,
    &TaskBH1750,
    0);

    xTaskCreatePinnedToCore(
    readRTCData,
    "TaskRTC",
    20000,
    NULL,
    1,
    &TaskRTC,
    0);

    //Inizializzazione Server Web
    AsyncWebServer server(1883);
    server.on("/", HTTP_GET, handleRoot);
    server.begin();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();

}
