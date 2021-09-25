#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include "generated.h"

/**
    Todo:
    - General brightness modifier on the app
    - Instead of setAll do FastLED.showColor
    - make a "custom" with a sort of all included - change all sorts of things based on all sorts of different functions (aka, value set to sin, brightness set to abs, of time variable, with x scaling as well.
*/

#define STASSID "umd-iot"
#define STAPSK  "2kbgxi8svgye"
#define ENABLE_WIFI
//#define ENABLE_INTRO
//#define ENABLE_FRAME_WAIT_FOR_KEY
#define REFRESH_RATE 60 //Change this and animations get fucked.
//#define TIME_DEBUG
#define timeevent(label) {};
#ifdef TIME_DEBUG
unsigned long start;
#undef timeevent(label)
#define timeevent(label) {Serial.print(label " :"); Serial.println(micros()-start); start = micros();}
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

void handleNotFound() {
    String message = F("File Not Found\n\n");
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, F("text/plain"), message);
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("Starting program!");
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
#endif
    Serial.println("");
    Serial.print(F("Connected to "));
    Serial.println(ssid);
    Serial.print(F("Server starting on: "));
    Serial.print(WiFi.localIP());
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
        fillInArgs(selected, server);
        server.send(200, F("text/plain"), F("Set selected mode"));
    });

    server.on("/gt", []() {
        server.send(200, F("text/plain"), String(1000000.0  /lastRunTime));
    });

    server.onNotFound(handleNotFound);
#ifdef ENABLE_WIFI
    server.begin();
#endif
    Serial.println(F("Server started! We are live!"));
    runInitLEDS();
}

//Q: Do we want to write a new frame every time this function is called?
unsigned long nextRun = 0;
void loop(void) {
#ifdef ENABLE_FRAME_WAIT_FOR_KEY
    while (!Serial.available()) {
        yield();
        server.handleClient();
    }
    while (Serial.available()) Serial.read();
#endif
    server.handleClient();
    if (millis() - nextRun > 1000 / REFRESH_RATE) {
        nextRun += 1000 / REFRESH_RATE;
        unsigned long start1 = micros();
        runLEDs();
        lastRunTime = micros() - start1;
    }
    yield();
}
