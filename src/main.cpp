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
const char* PARAM_INPUT_TEMP = "temp_setting";
//Variables to save values from HTML form
String temp_setting;
// File paths to save input values permanently
const char* tempPath = "/temp_setting.txt";

//PowerState Parameters
int powerGPIO = D4;

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
  readings["temperature"] = String(bme.readTemperature() - 1);
  readings["humidity"] = String(bme.readHumidity());
  readings["pressure"] = String(bme.readPressure()/100.0F);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Get Dashboard Inputs and return JSON object
String getCurrentInputValues(){
  values["temp_setting"] = temp_setting;
  String jsonString = JSON.stringify(values);
  return jsonString;
}

// Return JSON with Current Power State
String getPowerState(){
  JSONVar state;
  state["power"]["state"] = String(digitalRead(powerGPIO));
  String jsonString = JSON.stringify(state);
  Serial.print(jsonString);
  return jsonString;
}

void setup() {
  
  Serial.begin(115000);
  initBME();
  initFS();
  initWiFi();

  // Load values saved in LittleFS
  temp_setting = readFile(LittleFS, tempPath);

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
    Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

   server.serveStatic("/", LittleFS, "/");

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  // Web Server Root POST method for recieveing input temp_parameter from html
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    AsyncWebParameter* p = request->getParam(0);
    if(p->isPost()){
      // HTTP POST input value
      if (p->name() == PARAM_INPUT_TEMP) {
        temp_setting = p->value().c_str();
        Serial.print("input_temp set to: ");
        Serial.println(temp_setting);
        // Write file to save value
        writeFile(LittleFS, tempPath, temp_setting.c_str());
      }
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    request->send(LittleFS, "/index.html", "text/html");
  });

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  // Request for the latest input values
  server.on("/values", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getCurrentInputValues();
    request->send(200, "application/json", json);
    json = String();
  });

  // Request for the latest power state
  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getPowerState();
    request->send(200, "application/json", json);
    json = String();
  });

  //GET request to /power?state=<state>
  server.on("/power", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String state;
    // GET input value on /power?state=<state>
    if (request->hasParam("state")) {
      state = request->getParam("state")->value();
      // Control GPIO
      digitalWrite(powerGPIO, state.toInt());
    }
    else {
      state = "No message sent";
    }
    Serial.print("Power set to: ");
    Serial.println(state);
    request->send(200, "text/plain", "OK");
  });

  server.addHandler(&events);

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  // Set Power state
  pinMode(powerGPIO, OUTPUT);
}
void loop() {
  AsyncElegantOTA.loop();
  if ((millis() - lastTime) > timerDelay) {
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
  }
}
