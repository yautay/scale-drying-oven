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


AsyncWebServer server(80);
AsyncEventSource events("/events");

JSONVar readings;
JSONVar settings;

unsigned long lastTime = 0;

//Variables to save values from HTML form
String temp_setting;
String bed_setting;
String power_setting;

// File paths to save input values permanently
const char* temp_path = "/temp_setting.txt";
const char* settings_path = "/state_setting.txt";

Adafruit_BME280 bme;

// Initialize LittleFS
void initFS(){
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
void initWiFi(){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
}

// Init BME280
void initBME(){
    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);}
}

// Return bed temperature
int getBedADCValue() {
    int adc_value = analogRead(A0);
    return adc_value;
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
    readings["temperature"] = String(bme.readTemperature() - 1);
    readings["humidity"] = String(bme.readHumidity());
    readings["pressure"] = String(bme.readPressure()/100.0F);
    readings["bed_adc"] = String(getBedADCValue());
    String jsonString = JSON.stringify(readings);
    Serial.println(jsonString);
    return jsonString;
}

// Get Dashboard Inputs and return JSON object
String getCurrentSettingsValues(){
    settings["temp_setting"] = temp_setting;
    settings["bed_setting"] = bed_setting;
    settings["power_setting"] = power_setting;
    String jsonString = JSON.stringify(settings);
    Serial.println(jsonString);
    return jsonString;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);

    Serial.begin(115000);
    initBME();
    initFS();
    initWiFi();

    // Load values saved in LittleFS
    temp_setting = readFile(LittleFS, temp_path);
    power_setting = readFile(LittleFS, settings_path);

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
        AsyncWebParameter *p = request->getParam(0);
        if (p->isPost()) {
            // HTTP POST input value
            if (p->name() == "temp_setting") {
                temp_setting = p->value().c_str();
                Serial.println("input_temp set to: " + temp_setting);
                // Write file to save value
                writeFile(LittleFS, settings_path, temp_setting.c_str());
                Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());}
            if (p->name() == "bed_setting") {
                bed_setting = p->value().c_str();
                Serial.println("bed_temp set to: " + bed_setting);
                // Write file to save value
                writeFile(LittleFS, settings_path, bed_setting.c_str());
                Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());}
            if (p->name() == "power_setting") {
                power_setting = p->value().c_str();
                Serial.println("power set to: " + power_setting);
                // Write file to save value
                writeFile(LittleFS, settings_path, bed_setting.c_str());
                Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());}}});

    // Request for the latest sensor readings
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorReadings();
        request->send(200, "application/json", json);
        json = String();
    });

    // Request for the latest input values
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getCurrentSettingsValues();
        request->send(200, "application/json", json);
        json = String();
    });

    server.addHandler(&events);
    AsyncElegantOTA.begin(&server); // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started");
    }

    void loop(){

    AsyncElegantOTA.loop();

    if ((millis() - lastTime) > timerDelay) {
        events.send("ping",NULL, millis());
        events.send(getSensorReadings().c_str(),"new_readings" ,millis());
        events.send(getCurrentSettingsValues().c_str(), "new_settings", millis());
        lastTime = millis();

        if (power){
            Serial.println("power loop");
            if ((bme.readTemperature() - 1) <= (temp_setting.toFloat() + (histeresis / 2))){
                Serial.println("chamber temp to low");
                if (get_bed_temp(NTC) <= (bed_temp + 5)){
                    digitalWrite(HEATER_PIN, HIGH);
                    Serial.println("HEATING...");
                } else if (get_bed_temp(NTC) >= (bed_temp -5)){
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
