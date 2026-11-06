// WiFi pristupovy bod (AP) + webove rozhrani pro ovladani efektu.
//
// Pripoj se telefonem k WiFi "Eduingo-Logo" (heslo nize) a otevri
// http://192.168.4.1 (vetsina telefonu stranku otevre sama).

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "settings.h"
#include "index_html.h"

#define AP_SSID     "Eduingo-Logo"
#define AP_PASSWORD "eduingo123"   // min. 8 znaku; "" = otevrena sit
#define SAVE_DELAY_MS 3000         // ulozit do flash az chvili po posledni zmene

static WebServer server(80);
static DNSServer dns;
static bool saveDirty = false;
static uint32_t lastChange = 0;


static void sendState() {
    char color[8];
    snprintf(color, sizeof(color), "#%06lX", (unsigned long)(settings.color & 0xFFFFFF));
    String json;
    json.reserve(600);
    json += "{\"effect\":";   json += settings.effect;
    json += ",\"bri\":";      json += settings.brightness;
    json += ",\"speed\":";    json += settings.speed;
    json += ",\"color\":\"";  json += color; json += "\"";
    json += ",\"palette\":";  json += settings.palette ? "true" : "false";
    json += ",\"effects\":[";
    for (uint8_t i = 0; i < EFFECT_COUNT; i++) {
        if (i) json += ",";
        json += "\""; json += effects[i].name; json += "\"";
    }
    json += "]}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json; charset=utf-8", json);
}

static void handleSet() {
    bool restart = false;

    if (server.hasArg("effect")) {
        int e = server.arg("effect").toInt();
        if (e >= 0 && e < EFFECT_COUNT && e != settings.effect) {
            settings.effect = e;
            restart = true;
        }
    }
    if (server.hasArg("bri")) {
        settings.brightness = constrain(server.arg("bri").toInt(), 1, 255);
        applyBrightness();
    }
    if (server.hasArg("speed")) {
        settings.speed = constrain(server.arg("speed").toInt(), 10, 500);
    }
    if (server.hasArg("color")) {
        uint32_t c = strtoul(server.arg("color").c_str(), nullptr, 16) & 0xFFFFFF;
        if (c != settings.color) { settings.color = c; restart = true; }
    }
    if (server.hasArg("palette")) {
        bool p = server.arg("palette").toInt() != 0;
        if (p != settings.palette) { settings.palette = p; restart = true; }
    }

    if (restart) effectRestart = true;
    saveDirty = true;
    lastChange = millis();
    sendState();
}

// Jakakoli jina adresa -> presmerovat na hlavni stranku (captive portal)
static void handleNotFound() {
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
    server.send(302, "text/plain", "");
}

void webSetup() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    // LOLIN C3 mini ma znamy problem s antenou - nizsi vysilaci vykon
    // vyrazne zlepsuje stabilitu spojeni.
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    dns.start(53, "*", WiFi.softAPIP());

    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
    });
    server.on("/api/state", HTTP_GET, sendState);
    server.on("/api/set", HTTP_GET, handleSet);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.printf("WiFi AP \"%s\" bezi, ovladani na http://%s/\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
}

void webLoop() {
    dns.processNextRequest();
    server.handleClient();

    if (saveDirty && millis() - lastChange > SAVE_DELAY_MS) {
        saveDirty = false;
        saveSettings();
    }
}
