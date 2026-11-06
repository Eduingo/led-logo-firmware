// WiFi pristupovy bod (AP) + webove rozhrani pro ovladani efektu.
//
// Pripoj se telefonem k WiFi "Eduingo-Logo" (heslo nize) a otevri
// http://192.168.4.1 (vetsina telefonu stranku otevre sama).

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "settings.h"

#define AP_SSID     "Eduingo-Logo"
#define AP_PASSWORD "eduingo123"   // min. 8 znaku; "" = otevrena sit
#define SAVE_DELAY_MS 3000         // ulozit do flash az chvili po posledni zmene

static WebServer server(80);
static DNSServer dns;
static bool saveDirty = false;
static uint32_t lastChange = 0;

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="cs"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Eduingo logo</title>
<style>
:root{--bg:#111318;--card:#1c1f27;--fg:#eef0f4;--mut:#9aa1ad;--acc:#8b5cf6}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--fg)}
main{max-width:520px;margin:0 auto;padding:16px}
h1{font-size:22px;margin:4px 0 16px}
section{background:var(--card);border-radius:14px;padding:14px;margin-bottom:14px}
h2{font-size:13px;text-transform:uppercase;letter-spacing:.06em;color:var(--mut);margin:0 0 10px}
.fx{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.fx button{padding:12px 8px;border:1px solid #2c3140;border-radius:10px;background:#242834;color:var(--fg);font-size:15px}
.fx button.on{background:var(--acc);border-color:var(--acc)}
label{display:flex;justify-content:space-between;font-size:15px;margin:6px 0}
input[type=range]{width:100%;accent-color:var(--acc)}
.row{display:flex;align-items:center;gap:10px;flex-wrap:wrap}
input[type=color]{width:56px;height:44px;border:none;background:none;padding:0}
.sw{width:34px;height:34px;border-radius:50%;border:2px solid #0006}
.tg{display:flex;align-items:center;gap:8px;margin-top:12px;font-size:15px}
.tg input{width:20px;height:20px;accent-color:var(--acc)}
#st{font-size:13px;color:var(--mut);text-align:center}
</style></head><body><main>
<h1>Eduingo logo</h1>
<section><h2>Efekt</h2><div class="fx" id="fx"></div></section>
<section><h2>Parametry</h2>
<label>Jas <span id="briV"></span></label><input type="range" id="bri" min="1" max="255">
<label>Rychlost <span id="spdV"></span></label><input type="range" id="speed" min="10" max="500" step="5">
</section>
<section><h2>Barva</h2>
<div class="row"><input type="color" id="color"><div class="row" id="sw"></div></div>
<label class="tg"><input type="checkbox" id="palette">Pestrobarevná paleta (jinak jen zvolená barva)</label>
</section>
<p id="st"></p>
</main><script>
const $=id=>document.getElementById(id);
const SW=['#ff0000','#ff7a00','#ffff00','#00ff00','#00ffff','#0000ff','#640a82','#ff00ff','#ffffff'];
let st={},tm={};
function render(){
 $('fx').innerHTML='';
 st.effects.forEach((n,i)=>{const b=document.createElement('button');b.textContent=n;
  if(i==st.effect)b.className='on';b.onclick=()=>send({effect:i});$('fx').appendChild(b);});
 $('bri').value=st.bri;$('briV').textContent=Math.round(st.bri/2.55)+' %';
 $('speed').value=st.speed;$('spdV').textContent=st.speed+' %';
 $('color').value=st.color;$('palette').checked=st.palette;
}
function send(p,delay){
 const q=new URLSearchParams(p).toString();
 clearTimeout(tm[q.split('=')[0]]);
 tm[q.split('=')[0]]=setTimeout(()=>fetch('/api/set?'+q).then(r=>r.json()).then(s=>{st=s;render();$('st').textContent='';})
  .catch(()=>$('st').textContent='Nepodařilo se spojit s logem'),delay||0);
}
$('bri').oninput=e=>{$('briV').textContent=Math.round(e.target.value/2.55)+' %';send({bri:e.target.value},120)};
$('speed').oninput=e=>{$('spdV').textContent=e.target.value+' %';send({speed:e.target.value},120)};
$('color').oninput=e=>send({color:e.target.value.slice(1)},150);
$('palette').onchange=e=>send({palette:e.target.checked?1:0});
SW.forEach(c=>{const d=document.createElement('button');d.className='sw';d.style.background=c;
 d.onclick=()=>send({color:c.slice(1)});$('sw').appendChild(d);});
fetch('/api/state').then(r=>r.json()).then(s=>{st=s;render();});
</script></body></html>)HTML";

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
