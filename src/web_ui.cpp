// WiFi (vlastni AP + volitelne pripojeni k domaci siti) a webove rozhrani.
//
// Logo vzdy vytvari vlastni WiFi "Eduingo-Logo" (ovladani na http://192.168.4.1).
// Kdyz se v rozhrani nastavi domaci sit, logo se k ni navic pripoji a je
// dostupne na http://logo.local (mDNS) nebo na IP adrese od routeru.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "settings.h"
#include "index_html.h"

#define AP_SSID       "Eduingo-Logo"
#define AP_PASSWORD   "eduingo123"   // min. 8 znaku; "" = otevrena sit
#define HOSTNAME      "logo"         // => http://logo.local
#define SAVE_DELAY_MS 3000           // ulozit do flash az chvili po posledni zmene

static WebServer server(80);
static DNSServer dns;
static bool saveDirty = false;
static uint32_t lastChange = 0;

// Domaci sit
static String staSsid, staPass;
static volatile uint8_t staLastReason = 0;   // duvod posledniho odpojeni (0 = zadny)
static uint32_t connectAt = 0;               // kdy spustit pripojeni (po odeslani odpovedi)

// Escapovani textu do JSON (nazvy siti muzou obsahovat cokoli)
static void jsonStr(String &out, const String &s) {
    out += '"';
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if ((uint8_t)c < 0x20) { char b[8]; snprintf(b, sizeof(b), "\\u%04x", c); out += b; }
        else out += c;
    }
    out += '"';
}

static void sendJson(const String &json) {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json; charset=utf-8", json);
}

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
    sendJson(json);
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

// ---------------------------------------------------------------------------
// Domaci WiFi
// ---------------------------------------------------------------------------

static void loadWifi() {
    Preferences p;
    p.begin("wifi", true);
    staSsid = p.getString("ssid", "");
    staPass = p.getString("pass", "");
    p.end();
}

static void saveWifi() {
    Preferences p;
    p.begin("wifi", false);
    p.putString("ssid", staSsid);
    p.putString("pass", staPass);
    p.end();
}

static void startSta() {
    staLastReason = 0;
    WiFi.disconnect(false, true);
    if (staSsid.length()) {
        WiFi.begin(staSsid.c_str(), staPass.c_str());
        Serial.printf("Pripojuji se k \"%s\"...\n", staSsid.c_str());
    }
}

static const char *staStatus() {
    if (!staSsid.length()) return "none";
    if (WiFi.status() == WL_CONNECTED) return "connected";
    switch (staLastReason) {
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_AUTH_EXPIRE:
            return "wrongpass";
        case WIFI_REASON_NO_AP_FOUND:
            return "notfound";
        default:
            return "connecting";
    }
}

static void sendWifi() {
    String json;
    json.reserve(200);
    json += "{\"ssid\":";   jsonStr(json, staSsid);
    json += ",\"status\":\""; json += staStatus(); json += "\"";
    json += ",\"ip\":\"";   json += WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String(""); json += "\"";
    json += ",\"rssi\":";   json += WiFi.status() == WL_CONNECTED ? (int)WiFi.RSSI() : 0;
    json += ",\"host\":\"" HOSTNAME ".local\"";
    json += ",\"ap\":";     jsonStr(json, String(AP_SSID));
    json += "}";
    sendJson(json);
}

// /api/scan?start=1 spusti hledani, dalsi volani /api/scan vrati vysledek
static void handleScan() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) { sendJson("{\"scanning\":true}"); return; }
    if (n < 0 || server.hasArg("start")) {
        // Kdyz se logo prave neuspesne pripojuje, hledani by selhavalo ->
        // pokusy na chvili zastavit a obnovit je pozdeji.
        if (staSsid.length() && WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect(false, false);
            connectAt = millis() + 20000;
        }
        WiFi.scanDelete();
        WiFi.scanNetworks(true);
        sendJson("{\"scanning\":true}");
        return;
    }
    String json = "{\"scanning\":false,\"nets\":[";
    int count = 0;
    for (int i = 0; i < n && count < 25; i++) {
        String ssid = WiFi.SSID(i);
        if (!ssid.length()) continue;                 // skryte site
        bool dup = false;                             // stejna sit z vice access pointu
        for (int j = 0; j < i; j++) if (WiFi.SSID(j) == ssid) { dup = true; break; }
        if (dup) continue;
        if (count++) json += ",";
        json += "{\"ssid\":"; jsonStr(json, ssid);
        json += ",\"rssi\":"; json += (int)WiFi.RSSI(i);
        json += ",\"open\":"; json += WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "true" : "false";
        json += "}";
    }
    json += "]}";
    WiFi.scanDelete();
    sendJson(json);
}

static void handleWifiSave() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    if (!ssid.length() || ssid.length() > 32 ||
        (pass.length() && (pass.length() < 8 || pass.length() > 63))) {
        server.send(400, "text/plain; charset=utf-8", "Neplatný název sítě nebo heslo");
        return;
    }
    staSsid = ssid;
    staPass = pass;
    saveWifi();
    sendWifi();
    connectAt = millis() + 300;   // pripojit az po odeslani odpovedi
}

static void handleWifiForget() {
    connectAt = 0;
    staSsid = "";
    staPass = "";
    saveWifi();
    WiFi.disconnect(false, true);
    sendWifi();
}

static void onWifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
        staLastReason = 0;
        Serial.printf("Pripojeno k domaci WiFi, IP %s, http://" HOSTNAME ".local/\n",
                      WiFi.localIP().toString().c_str());
    } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        staLastReason = info.wifi_sta_disconnected.reason;
    }
}

// Jakakoli jina adresa -> presmerovat na hlavni stranku (captive portal)
static void handleNotFound() {
    // Pres AP (captive portal) presmerovat na IP loga, z domaci site jen na "/"
    bool viaAp = server.client().localIP() == WiFi.softAPIP();
    server.sendHeader("Location", viaAp ? String("http://") + WiFi.softAPIP().toString() + "/" : String("/"), true);
    server.send(302, "text/plain", "");
}

void webSetup() {
    loadWifi();
    WiFi.persistent(false);          // prihlasovaci udaje si ukladame sami
    WiFi.onEvent(onWifiEvent);
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.setAutoReconnect(true);
    // LOLIN C3 mini ma znamy problem s antenou - nizsi vysilaci vykon
    // vyrazne zlepsuje stabilitu spojeni.
    WiFi.setTxPower(WIFI_POWER_8_5dBm);

    startSta();

    dns.start(53, "*", WiFi.softAPIP());
    if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);

    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
    });
    server.on("/api/state", HTTP_GET, sendState);
    server.on("/api/set", HTTP_GET, handleSet);
    server.on("/api/wifi", HTTP_GET, sendWifi);
    server.on("/api/wifi", HTTP_POST, handleWifiSave);
    server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
    server.on("/api/scan", HTTP_GET, handleScan);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.printf("WiFi AP \"%s\" bezi, ovladani na http://%s/\n",
                  AP_SSID, WiFi.softAPIP().toString().c_str());
}

void webLoop() {
    dns.processNextRequest();
    server.handleClient();

    if (connectAt && (int32_t)(millis() - connectAt) >= 0) {
        connectAt = 0;
        startSta();
    }

    if (saveDirty && millis() - lastChange > SAVE_DELAY_MS) {
        saveDirty = false;
        saveSettings();
    }
}
