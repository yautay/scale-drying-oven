#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "LittleFS.h"

#include <Arduino_JSON.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "credentials.h"
#include "config.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

AsyncWebServer server(80);
AsyncEventSource events("/events");

JSONVar readings;
JSONVar values;

unsigned long lastTime = 0;
unsigned long timerDelay = 1500;

Adafruit_BME280 bme;

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_TEMP = "input_temp";
const char* PARAM_INPUT_SWITCH = "input_switch";
//Variables to save values from HTML form
String input_temp;
String input_switch;
// File paths to save input values permanently
const char* tempPath = "/inputTemp.txt";
const char* switchPath = "/inputSwitch.txt";

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
  return String();
  }
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Init BME280
void initBME(){
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["temperature"] = String(bme.readTemperature() - 2);
  readings["humidity"] = String(bme.readHumidity());
  readings["pressure"] = String(bme.readPressure()/100.0F);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Get Dashboard Inputs and return JSON object
String getCurrentInputValues(){
  values["input_temp"] = input_temp;
  values["input_switch"] = input_switch;
  String jsonString = JSON.stringify(values);
  return jsonString;
}

void setup() {
  
  Serial.begin(115000);
  initBME();
  initFS();
  initWiFi();

  // Load values saved in LittleFS
  input_temp = readFile(LittleFS, tempPath);
  input_switch = readFile(LittleFS, switchPath);

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  server.on("/values", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getCurrentInputValues();
    request->send(200, "application/json", json);
    json = String();
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST input1 value
        if (p->name() == PARAM_INPUT_TEMP) {
          input_temp = p->value().c_str();
          Serial.print("input_temp set to: ");
          Serial.println(input_temp);
          // Write file to save value
          writeFile(LittleFS, tempPath, input_temp.c_str());
        }
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.addHandler(&events);

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  AsyncElegantOTA.loop();
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 30 seconds
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}
