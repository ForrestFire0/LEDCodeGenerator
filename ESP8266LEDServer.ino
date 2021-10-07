#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include "generated.h"

/**
    Todo:
    - make a "custom" with a sort of all included - change all sorts of things based on all sorts of different functions (aka, value set to sin, brightness set to abs, of time variable, with x scaling as well.
*/

#define STASSID "umd-iot"
#define STAPSK  "2kbgxi8svgye"
#define ENABLE_WIFI
//#define ENABLE_INTRO
//#define ENABLE_FRAME_WAIT_FOR_KEY
//#define TIME_DEBUG


#define timeevent(label) {};
#define start() {};
#ifdef TIME_DEBUG
unsigned long start;
#undef timeevent(label)
#undef start();
#define timeevent(label) {Serial.print(label ": "); Serial.println(micros()-start); start = micros();}
#define start() {start = micros();}
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

Mode selected = OFF;

ESP8266WebServer server(80);

unsigned long lastRunTime;

void handleRoot() {
    server.send(200, F("text/html"), HTMLTemplate);
    Serial.println(F("Responding and sending"));
}

void setup(void) {
    Serial.begin(115200);
    startSelected();
    startLEDs();
#ifdef ENABLE_WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    //Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println("Starting program!");
#endif
    server.on("/", handleRoot);
    server.on("/set", []() {
        Mode oldSelected = selected;
        selected = (Mode) server.arg(0).toInt();
        if (selected != oldSelected) {
            endSelected(oldSelected);
            Serial.print(F("Set mode: "));
            Serial.println(LEDOptions[(byte) selected]);
            startSelected();
        }
        Serial.println("Set parameters:");
        fillInArgs(selected, server);
        stringifyParams(selected);
        Serial.println(spBuffer);
        server.send(200, F("text/plain"), F("Done!"));
    });

    server.on("/gt", []() {
        sprintf(spBuffer, "%f", 1000000.0  / lastRunTime);
        server.send(200, F("text/plain"), spBuffer);
    });

    server.on("/gc", []() {
        stringifyParams(selected);
        Serial.println(spBuffer);
        server.send(200, F("text/plain"), spBuffer);
    });

#ifdef ENABLE_WIFI
    server.begin();
#endif
    Serial.print(F("Connected to "));
    Serial.println(ssid);
    Serial.print(F("Server starting on: "));
    Serial.println(WiFi.localIP());
    runInitLEDS();
}

//Q: Do we want to write a new frame every time this function is called?
void loop(void) {
#ifdef ENABLE_FRAME_WAIT_FOR_KEY
    while (!Serial.available()) {
        yield();
        server.handleClient();
    }
    while (Serial.available()) Serial.read();
#endif
    unsigned long s = micros();
    server.handleClient();
    runLEDs();
    yield();
    while(micros() - s < 10000) yield();
    lastRunTime = micros() - s;
}
