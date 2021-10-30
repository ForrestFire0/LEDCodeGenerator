#include <helpers.h>
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
//#define ENABLE_FRAME_WAIT_FOR_KEY
//#define TIME_DEBUG


#define timeevent(label) {};
#define start() {};
#ifdef TIME_DEBUG
unsigned long start;
#undef timeevent(label)
#undef start();
#define timeevent(label) {p(label ": "); Serial.println(micros()-start); start = micros();}
#define start() {start = micros();}
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

Mode selected = OFF;

ESP8266WebServer server(80);

unsigned long lastRunTime;

void setup(void) {
    Serial.begin(115200);
    psl("Starting program!");
    startSelected();
#ifdef ENABLE_WIFI
    WiFi.begin(ssid, password);
//    IPAddress local_IP(10,108,64,223);
//    IPAddress gateway(10, 108, 0, 1);
//    IPAddress subnet(255, 255, 128, 0);
//        if (!WiFi.config(local_IP, gateway, subnet)) {
//            Serial.println("STA Failed to configure");
//            setAll(CRGB::Green);
//            FastLED.show();
//            delay(500);
//            setAll(CRGB::Black);
//            FastLED.show();
//        }
    startLEDs();
#endif
    server.on("/", []() {
        server.send(200, F("text/html"), HTMLTemplate);
        psl("Responding and sending");
    });
    server.on("/set", []() {
        Mode oldSelected = selected;
        selected = (Mode) server.arg(0).toInt();
        if (selected != oldSelected) {
            endSelected(oldSelected);
            ps("Set mode: ");
            pl(LEDOptions[(byte) selected]);
            startSelected();
        }
        ps("Set parameters:");
        fillInArgs(selected, server);
        stringifyParams(selected);
        pl(spBuffer);
        server.send(200, F("text/plain"), F("Done!"));
    });

    server.on("/gt", []() {
        sprintf(spBuffer, "%f", 1000000.0  / lastRunTime);
        server.send(200, F("text/plain"), spBuffer);
    });

    server.on("/gc", []() {
        stringifyParams(selected);
        //        Serial.println(spBuffer);
        server.send(200, F("text/plain"), spBuffer);
    });

#ifdef ENABLE_WIFI
    server.begin();
#endif
    p(F("Connected to "));
    Serial.println(ssid);
    p(F("Server starting on: "));
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
    while (micros() - s < 10000) yield();
    lastRunTime = micros() - s;
}
