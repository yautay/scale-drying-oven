#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "config.h"

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";

// IPAddress localIP;
IPAddress localIP(192, 168, 2, 200); // hardcoded
// Gateway IP address
IPAddress gateway(192, 168, 2, 90);
IPAddress subnet(255, 255, 0, 0);

AsyncWebServer server(80);
AsyncEventSource events("/events");

JSONVar readings;
JSONVar values;
JSONVar state;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000;
unsigned int bed_temp = 60;
unsigned int histeresis = 1;
boolean restart = false;
bool power;

//Variables to save values from HTML form
String temp_setting;
String power_setting;
String ssid;
String pass;
String ip;

// File paths to save input values permanently
const char* tempPath = "/temp_setting.txt";
const char* statePath = "/state_setting.txt";
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";

Adafruit_BME280 bme;

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
  file.close();
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
  file.close();
}

// Initialize WiFi
bool initWiFi() {
  int connect_attempt = 0;
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }
  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  if (!WiFi.config(localIP, gateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");
  do {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
      connect_attempt++;
    } else break;
  } while (connect_attempt < 20);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Failed to connect.");
    return false;
  }
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
  const double VCC = 3.3;             // NodeMCU on board 3.3v vcc
  const double R2 = NTC;            // 105k ohm series resistor
  const double adc_resolution = 1023; // 10-bit adc

  const double A = 0.001129148;   // thermistor equation parameters
  const double B = 0.000234125;
  const double C = 0.0000000876741; 

  double Vout, Rth, temperature, adc_value; 

  adc_value = analogRead(A0);
  Vout = (adc_value * VCC) / adc_resolution;
  Rth = (VCC * R2 / Vout) - R2;

/*  Steinhart-Hart Thermistor Equation:
 *  Temperature in Kelvin = 1 / (A + B[ln(R)] + C[ln(R)]^3)
 *  where A = 0.001129148, B = 0.000234125 and C = 8.76741*10^-8  */
  temperature = (1 / (A + (B * log(Rth)) + (C * pow((log(Rth)),3))));   // Temperature in kelvin
  temperature = temperature - 273.15;  // Temperature in degree celsius
  
  Serial.print("Bed temp: ");
  Serial.print(temperature);
  Serial.print(" C \n");
  return temperature;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);

  Serial.begin(115000);
  initBME();
  initFS();
  initWiFi();

  // Load values saved in LittleFS
  temp_setting = readFile(LittleFS, tempPath);
  power_setting = readFile(LittleFS, statePath);
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);


  if(initWiFi()) {
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
  } 
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      restart = true;
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    });
    server.begin();
  }
}
void loop() {
  if(restart){
    delay(5000);
    ESP.restart();
  }
  AsyncElegantOTA.loop();
  if ((millis() - lastTime) > timerDelay) {
    events.send("ping",NULL,millis());
    events.send(getSensorReadings().c_str(),"new_readings" ,millis());
    lastTime = millis();
    if (power){
      Serial.println("power loop");
      if ((bme.readTemperature() - 1) <= (temp_setting.toFloat() + (histeresis / 2))){
        Serial.println("chamber temp to low");
        if (get_bed_temp(NTC) <= (bed_temp + 3)){
          digitalWrite(HEATER_PIN, HIGH);
          Serial.println("HEATING...");
        } else if (get_bed_temp(NTC) >= (bed_temp -3)){
          digitalWrite(HEATER_PIN, LOW);
          Serial.println("BED TO HOT...");
        }
      } else if ((bme.readTemperature() - 1) >= (temp_setting.toFloat() - (histeresis / 2))){
        digitalWrite(HEATER_PIN, LOW);
        Serial.println("chamber temp to high");
      }
    } else {
      digitalWrite(HEATER_PIN, LOW);
    }
  }
}
