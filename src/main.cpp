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
JSONVar state;

unsigned long lastTime = 0;
unsigned long timerDelay = 500;
unsigned int bed_temp = 60;
unsigned int histeresis = 1;

bool power;

Adafruit_BME280 bme;

//Variables to save values from HTML form
String temp_setting;
String power_setting;
// File paths to save input values permanently
const char* tempPath = "/temp_setting.txt";
const char* statePath = "/state_setting.txt";

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

// Return Current Power State set power variable and return JSON object
String getPowerState(){
  if (power_setting == "1"){
    power = true;
  } else {
    power = false;
  }
  state["state"] = power_setting;
  String jsonString = JSON.stringify(state);
  Serial.print(jsonString);
  return jsonString;
}

// Return bed temperature
float get_bed_temp(int thermistor_resistance) {
  int Vo;
  float R1 = thermistor_resistance;
  float logR2, R2, T;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

  Vo = analogRead(NTC_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  Serial.print("Bed temp: ");
  Serial.print(T);
  Serial.print(" C \n");
  return T;
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(NTC_PIN, INPUT);

  Serial.begin(115000);
  initBME();
  initFS();
  initWiFi();

  // Load values saved in LittleFS
  temp_setting = readFile(LittleFS, tempPath);
  power_setting = readFile(LittleFS, statePath);

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
    AsyncWebParameter* p = request->getParam(0);
    if(p->isPost()){
      // HTTP POST input value
      if (p->name() == "temp_setting") {
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
      if (state == "1"){
        power = true;
        digitalWrite(LED_BUILTIN, LOW);
      } else if (state == "0") {
        power = false;
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
    else {
      state = "No message sent";
    }
    Serial.print("Power set to: ");
    Serial.println(state);
    power_setting = state;
    // Write file to save state
    writeFile(LittleFS, statePath, state.c_str());
    request->send(200, "text/plain", "OK");
  });

  server.addHandler(&events);

  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  // Set Power state
  // pinMode(powerGPIO, OUTPUT);
}
void loop() {
  AsyncElegantOTA.loop();
  if ((millis() - lastTime) > timerDelay) {
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
    if (power){
      if ((bme.readTemperature() - 1) <= power_setting.toFloat() + (histeresis / 2)){
        if (get_bed_temp(NTC) <= (bed_temp + 3)){
          digitalWrite(HEATER_PIN, HIGH);
        } else if (get_bed_temp(NTC) >= (bed_temp -3)){
          digitalWrite(HEATER_PIN, HIGH);
        }
      } else if ((bme.readTemperature() - 1) >= power_setting.toFloat() - (histeresis / 2)){
        digitalWrite(HEATER_PIN, LOW);
      }
    } else {
      digitalWrite(HEATER_PIN, LOW);
    }
  }
}
