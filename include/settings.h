#pragma once
#include <Arduino.h>

// Nastaveni, ktera jdou menit z webu (a ukladaji se do flash).
struct Settings {
    uint8_t  effect;      // index efektu v tabulce efektu
    uint8_t  brightness;  // 1-255
    uint16_t speed;       // rychlost v %, 10-500 (100 = puvodni rychlost)
    uint32_t color;       // hlavni barva 0xRRGGBB
    bool     palette;     // true = pestrobarevna paleta, false = jen hlavni barva
    uint16_t autoCycle;   // automaticke stridani efektu po N sekundach (0 = vypnuto)
};

extern Settings settings;

// Nastavi se, kdyz je potreba prerusit bezici efekt (zmena efektu/barvy).
extern volatile bool effectRestart;

struct EffectInfo {
    const char *name;
    void (*run)();
};
extern const EffectInfo effects[];
extern const uint8_t EFFECT_COUNT;
extern const uint8_t EFFECT_OFF;
extern const uint8_t EFFECT_PAINT;

// Malovani stetcem
void paintSet(uint16_t led, uint32_t color);
void paintFill(uint32_t color);

// Casovac vypnuti (0 = nebezi), hodnota millis(), kdy se logo vypne
extern uint32_t sleepAt;

// Pro zivy nahled ve webu
String layoutJson();   // [[led,x,y],...] podle rucniho mapovani
String ledsHex();      // aktualni barvy vsech LED jako hex RRGGBB...

void applyBrightness();
void saveSettings();

// web_ui.cpp
void webSetup();
void webLoop();
