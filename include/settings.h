#pragma once
#include <Arduino.h>

// Nastaveni, ktera jdou menit z webu (a ukladaji se do flash).
struct Settings {
    uint8_t  effect;      // index efektu v tabulce efektu
    uint8_t  brightness;  // 1-255
    uint16_t speed;       // rychlost v %, 10-500 (100 = puvodni rychlost)
    uint32_t color;       // hlavni barva 0xRRGGBB
    bool     palette;     // true = pestrobarevna paleta, false = jen hlavni barva
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

void applyBrightness();
void saveSettings();

// web_ui.cpp
void webSetup();
void webLoop();
