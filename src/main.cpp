#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include "settings.h"

#define LED_PIN    4    // datovy pin pasku
#define LED_COUNT  217  // pocet LED na pasku
// Pozor na odber: 217 LED na plnou bilou muze teoreticky brat az ~13 A.
// Pokud zdroj nestaci (bliká, resetuje se ESP), sniz jas napr. na 80-120.
#define BRIGHTNESS 255  // vychozi jas (pak jde menit z webu)

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// Rucni mapovani LED: [znak] [sloupec] [LED ve sloupci]
// Cisla jsou 1-based (index na pasku = cislo - 1), 0 = konec sloupce/znaku.
// const => tabulka se uklada do flash misto do RAM.
#define SEG_CHARS 8
#define SEG_COLS  20
#define SEG_ROWS  8
const uint16_t segments[SEG_CHARS][SEG_COLS][SEG_ROWS] = {
    // { 21, 20, 22, 19, 23, 18, 24, 8, 9, 17, 25, 7, 10, 16, 26, 6, 11, 15, 27, 5, 12, 14, 28, 4, 13, 29, 213, 3, 14, 30, 214, 212, 2, 15, 31, 215, 211, 1, 16, 216, 210, 0, 32, 209, 33, 208, 34 },

    { // Logo
        { 23, 22, 21 },
        { 24, 18, 19, 20 },
        { 25, 17, 10, 9, 8 },
        { 26, 16, 11, 7 },
        { 27, 15, 12, 6 },
        { 28, 14, 13, 5 },
        { 29, 212, 213, 4 },
        { 30, 211, 214, 3 },
        { 31, 210, 215, 2 },
        { 32, 209, 216, 1 },
        { 33, 208 },
        { 34, 35 },
    },
    { // Char "E"
        { 36, 207, 206, 205 },
        { 37, 45, 51, 204 },
        { 38, 44, 46, 50, 52, 203 },
        { 39, 43, 47, 49, 53, 202 },
        { 40, 42, 48, 54, 201 },
        { 41, 200 },
    },
    { // Char "D"
        { 55, 56, 57 },
        { 58, 69, 70, 199 },
        { 59, 68, 71, 198 },
        { 60, 67, 72, 197 },
        { 62, 61, 74, 73 },
        { 63, 66, 75, 196 },
        { 64, 65, 76, 195 },
        { 77, 194 },
    },
    { // Char "U"
        { 80, 79, 78, 193 },
        { 81, 82, 83, 192 },
        { 84 },
        { 191 },
        { 85 },
        { 190 },
        { 88, 87, 86 },
        { 89, 189 },
        { 90, 187, 188 },
        { 91, 186 },
    },
    { // Char "I"
        { 94, 93, 92, 185, 184 },
        { 95, 183 },
        { 96, 97, 98, 99, 100, 182 },
    },
    { // Char "N"
        { 103, 102, 101, 181, 180 },
        { 104, 176, 177, 178, 179 },
        { 105 },
        { 175 },
        { 106 },
        { 107, 174, 173, 172, 171 },
        { 108, 109, 170, 168, 169 },
    },
    { // Char "G"
        { 110, 167, 159 },
        { 111, 119, 120, 166, 160, 156 },
        { 112, 118, 121, 165, 161, 158 },
        { 113, 117, 122, 164, 162, 157 },
        { 114, 116, 125, 124, 123, 163, 155 },
        { 115, 126, 151, 152, 153, 154 },
    },
    { // Char "O"
        { 128, 127, 150 },
        { 129, 136, 137, 149 },
        { 130, 135, 138, 148 },
        { 131, 134, 139, 147 },
        { 132, 133, 142, 141, 140, 146 },
        { 143, 144, 145 },
    }
};

// ---------------------------------------------------------------------------
// Nastaveni ovladana z webu
// ---------------------------------------------------------------------------
Settings settings = {
    0,                // efekt: Show (puvodni sekvence)
    BRIGHTNESS,
    100,              // rychlost 100 % = puvodni casovani
    0x640A82,         // fialova (100, 10, 130)
    true,             // pestrobarevna paleta
    0,                // automaticke stridani vypnuto
};
uint32_t sleepAt = 0;
static uint32_t effectStart = 0;
volatile bool effectRestart = false;

static Preferences prefs;

void applyBrightness() {
    strip.setBrightness(settings.brightness);
}

void loadSettings() {
    prefs.begin("logo", true);
    settings.effect     = prefs.getUChar("effect", settings.effect);
    settings.brightness = prefs.getUChar("bri", settings.brightness);
    settings.speed      = prefs.getUShort("speed", settings.speed);
    settings.color      = prefs.getULong("color", settings.color);
    settings.palette    = prefs.getBool("palette", settings.palette);
    settings.autoCycle  = prefs.getUShort("auto", settings.autoCycle);
    prefs.end();
    if (settings.effect >= EFFECT_COUNT) settings.effect = 0;
    if (settings.brightness == 0) settings.brightness = 1;
    settings.speed = constrain(settings.speed, 10, 500);
}

void saveSettings() {
    prefs.begin("logo", false);
    prefs.putUChar("effect", settings.effect);
    prefs.putUChar("bri", settings.brightness);
    prefs.putUShort("speed", settings.speed);
    prefs.putULong("color", settings.color);
    prefs.putBool("palette", settings.palette);
    prefs.putUShort("auto", settings.autoCycle);
    prefs.end();
}

// Nahrada za delay(): ceka (podle nastavene rychlosti), mezitim obsluhuje web.
// Vraci false, pokud se ma bezici efekt ukoncit (uzivatel zmenil efekt/barvu).
// Automatika: casovac vypnuti a stridani efektu
void automationTick() {
    uint32_t now = millis();
    if (sleepAt && (int32_t)(now - sleepAt) >= 0) {
        sleepAt = 0;
        settings.effect = EFFECT_OFF;
        effectRestart = true;
        return;
    }
    if (settings.autoCycle && settings.effect != EFFECT_OFF && settings.effect != 0 &&
        now - effectStart >= (uint32_t)settings.autoCycle * 1000) {
        // dalsi efekt; preskocit "Celou show" (ta stridani dela sama) a "Vypnuto"
        uint8_t e = settings.effect;
        do { e = (e + 1) % EFFECT_COUNT; } while (e == 0 || e == EFFECT_OFF);
        settings.effect = e;
        effectRestart = true;
    }
}

bool waitMs(uint32_t ms) {
    uint32_t scaled = (uint32_t)((uint64_t)ms * 100 / settings.speed);
    uint32_t start = millis();
    do {
        webLoop();
        automationTick();
        if (effectRestart) return false;
        delay(1);
    } while (millis() - start < scaled);
    return !effectRestart;
}

// Prerusi efekt, pokud uzivatel neco zmenil
#define WAIT(ms) do { if (!waitMs(ms)) return; } while (0)
#define CHECK()  do { if (effectRestart) return; } while (0)

uint32_t Wheel(byte WheelPos);

// Puvodni paleta barev
const uint32_t PALETTE[] = {
    0xFF0000, // Red
    0x00FF00, // Green
    0x0000FF, // Blue
    0xFFFF00, // Yellow
    0x00FFFF, // Cyan
    0xFF00FF, // Magenta
    0xFFFFFF  // White
};
const size_t PALETTE_SIZE = sizeof(PALETTE) / sizeof(PALETTE[0]);

// Barva z palety, nebo hlavni barva, kdyz je paleta vypnuta
uint32_t paletteColor(size_t i) {
    return settings.palette ? PALETTE[i % PALETTE_SIZE] : settings.color;
}

// Pevna barva efektu, nebo hlavni barva, kdyz je paleta vypnuta
uint32_t fixedOrMain(uint32_t c) {
    return settings.palette ? c : settings.color;
}

// ---------------------------------------------------------------------------
// Zakladni operace nad rucnim mapovanim
// ---------------------------------------------------------------------------

// Rozsviti zadany sloupec ve vsech znacich (znak po znaku, s pauzou)
void glowupColumn(uint8_t column, uint32_t color, uint8_t wait) {
    if (column >= SEG_COLS) return;
    for (size_t s = 0; s < SEG_CHARS; s++)
    {
        if (segments[s][column][0] == 0) continue; // znak tento sloupec nema

        for (size_t j = 0; j < SEG_ROWS && segments[s][column][j] != 0; j++)
        {
            strip.setPixelColor(segments[s][column][j] - 1, color);
        }
        strip.show();
        WAIT(wait);
    }
}

// Zhasne zadany sloupec ve vsech znacich
void glowdownColumn(uint8_t column, uint8_t wait) {
    glowupColumn(column, 0, wait);
}

// Rozsviti cely znak sloupec po sloupci
void glowupSegment(uint8_t segment, uint32_t color, uint8_t wait) {
    if (segment >= SEG_CHARS) return;
    for (size_t i = 0; i < SEG_COLS && segments[segment][i][0] != 0; i++)
    {
        for (size_t j = 0; j < SEG_ROWS && segments[segment][i][j] != 0; j++)
        {
            strip.setPixelColor(segments[segment][i][j] - 1, color);
        }
        strip.show();
        WAIT(wait);
    }
}

// Zhasne cely znak sloupec po sloupci
void glowdownSegment(uint8_t segment, uint8_t wait) {
    glowupSegment(segment, 0, wait);
}

// Nastavi cely znak najednou (bez animace)
void fillSegment(uint8_t segment, uint32_t color) {
    if (segment >= SEG_CHARS) return;
    for (size_t i = 0; i < SEG_COLS && segments[segment][i][0] != 0; i++)
        for (size_t j = 0; j < SEG_ROWS && segments[segment][i][j] != 0; j++)
            strip.setPixelColor(segments[segment][i][j] - 1, color);
}

// ---------------------------------------------------------------------------
// Efekty (kazdy probehne jednou, loop() ho pak spousti dokola)
// ---------------------------------------------------------------------------

// Postupne rozsviceni a zhasnuti vsech znaku
void fxSegments() {
    for (size_t s = 0; s < SEG_CHARS; s++)
    {
        glowupSegment(s, paletteColor(s), 10);
        CHECK();
    }
    WAIT(2000);
    for (size_t s = 0; s < SEG_CHARS; s++)
    {
        glowdownSegment(s, 10);
        CHECK();
    }
    // LED c. 217 neni v mapovani, takze by po rainbowCycle zustala svitit -> zhasnout vse
    strip.clear();
    strip.show();
}

// Nahodne jiskry
void fxSparkles() {
    for (size_t i = 0; i < 25; i++)
    {
        uint16_t pixels[12];
        for (size_t k = 0; k < 12; k++)
        {
            pixels[k] = random(0, strip.numPixels());
            strip.setPixelColor(pixels[k], paletteColor(random(0, PALETTE_SIZE)));
        }
        strip.show();
        WAIT(30);

        for (size_t k = 0; k < 12; k++)
        {
            strip.setPixelColor(pixels[k], 0); // Off
        }
        strip.show();
    }
}

// Rozsviceni a zhasinani sloupec po sloupci
void fxColumns() {
    for (size_t col = 0; col < SEG_COLS; col++)
    {
        glowupColumn(col, settings.color, 5);
        CHECK();
    }
    WAIT(2000);
    for (size_t col = 0; col < SEG_COLS; col++)
    {
        glowdownColumn(col, 5);
        CHECK();
    }
    WAIT(1000);
}

// EDU / IN / GO po skupinach a nakonec logo
void fxLetters() {
    uint32_t c1 = fixedOrMain(0xFF00FF);
    uint32_t c2 = fixedOrMain(0x00FFFF);
    uint32_t c3 = fixedOrMain(0x00FF00);

    glowupSegment(1, c1, 50); CHECK(); // E
    glowupSegment(2, c1, 50); CHECK(); // D
    glowupSegment(3, c1, 50); CHECK(); // U
    WAIT(300);
    glowdownSegment(1, 50); CHECK(); // E
    glowdownSegment(2, 50); CHECK(); // D
    glowdownSegment(3, 50); CHECK(); // U
    glowupSegment(4, c2, 50); CHECK(); // I
    glowupSegment(5, c2, 50); CHECK(); // N
    WAIT(300);
    glowdownSegment(4, 50); CHECK(); // I
    glowdownSegment(5, 50); CHECK(); // N
    glowupSegment(6, c3, 50); CHECK(); // G
    glowupSegment(7, c3, 50); CHECK(); // O
    WAIT(300);
    glowupSegment(0, fixedOrMain(0xFFFFFF), 50); CHECK(); // LOGO
    WAIT(100);
    glowdownSegment(6, 50); CHECK(); // G
    glowdownSegment(7, 50); CHECK(); // O
}

// Duha rovnomerne rozlozena pres cely pasek
void fxRainbowCycle() {
    for (uint16_t j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        WAIT(1);
    }
}

void fxRainbow() {
    for (uint16_t j = 0; j < 256; j++) {
        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        strip.show();
        WAIT(20);
    }
}

// Postupne plneni barvou LED po LED, pak zhasnuti
void fxColorWipe() {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, settings.color);
        strip.show();
        WAIT(5);
    }
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, 0);
        strip.show();
        WAIT(5);
    }
}

// Divadelni "bezici svetla"
void fxTheaterChase() {
    for (int q = 0; q < 3; q++) {
        for (uint16_t i = 0; i < strip.numPixels(); i += 3) {
            strip.setPixelColor(i + q, settings.color);
        }
        strip.show();
        WAIT(50);
        for (uint16_t i = 0; i < strip.numPixels(); i += 3) {
            strip.setPixelColor(i + q, 0);
        }
    }
}

void fxTheaterChaseRainbow() {
    for (int j = 0; j < 256; j++) {
        for (int q = 0; q < 3; q++) {
            for (uint16_t i = 0; i < strip.numPixels(); i += 3) {
                strip.setPixelColor(i + q, Wheel((i + j) % 255));
            }
            strip.show();
            WAIT(50);
            for (uint16_t i = 0; i < strip.numPixels(); i += 3) {
                strip.setPixelColor(i + q, 0);
            }
        }
    }
}

// Kazdy znak jinou barvou, barvy se pomalu posouvaji
void fxLettersRainbow() {
    for (uint16_t j = 0; j < 256; j++) {
        for (size_t s = 0; s < SEG_CHARS; s++) {
            fillSegment(s, Wheel((s * 256 / SEG_CHARS + j) & 255));
        }
        strip.show();
        WAIT(20);
    }
}

// Cele logo jednou barvou (barvu a jas jde menit zive)
void fxSolid() {
    strip.fill(settings.color);
    strip.show();
    WAIT(50);
}

// "Dychani" hlavni barvou
void fxBreathe() {
    uint8_t r = settings.color >> 16, g = settings.color >> 8, b = settings.color;
    for (int step = 0; step < 200; step++) {
        // trojuhelnik 0..255..0, zkvadraticky pro prirozenejsi nabeh
        int lin = step < 100 ? step : 200 - step;
        uint16_t k = (uint16_t)lin * lin * 255 / 10000;
        strip.fill(strip.Color(r * k / 255, g * k / 255, b * k / 255));
        strip.show();
        WAIT(15);
    }
}

// Puvodni kompletni show: vsechny efekty za sebou
void fxShow() {
    fxSegments(); CHECK();
    fxSparkles(); CHECK();
    fxColumns(); CHECK();
    fxLetters(); CHECK();
    // rozsvit vsechny znaky bile, pak duha
    for (size_t i = 1; i <= 7; i++)
    {
        glowupSegment(i, fixedOrMain(0xFFFFFF), 20);
        CHECK();
    }
    fxRainbowCycle();
}

void fxOff() {
    strip.clear();
    strip.show();
    WAIT(100);
}


// ---------------------------------------------------------------------------
// Rozlozeni LED v rovine (odvozene z rucniho mapovani): x = sloupec napric
// celym napisem (znaky vedle sebe, mezi nimi mezera), y = poradi ve sloupci.
// ---------------------------------------------------------------------------
static int8_t ledX[LED_COUNT];   // -1 = LED neni v mapovani
static int8_t ledY[LED_COUNT];
static uint8_t layoutW = 1;      // pocet sloupcu celkem

void buildLayout() {
    memset(ledX, -1, sizeof(ledX));
    memset(ledY, -1, sizeof(ledY));
    uint8_t x = 0;
    for (size_t s = 0; s < SEG_CHARS; s++) {
        size_t i = 0;
        for (; i < SEG_COLS && segments[s][i][0] != 0; i++) {
            for (size_t j = 0; j < SEG_ROWS && segments[s][i][j] != 0; j++) {
                uint16_t led = segments[s][i][j] - 1;
                if (led < LED_COUNT) { ledX[led] = x + i; ledY[led] = j; }
            }
        }
        x += i + 1;   // mezera mezi znaky
    }
    layoutW = x > 1 ? x - 1 : 1;
}

String layoutJson() {
    String out;
    out.reserve(LED_COUNT * 12);
    out += "{\"w\":"; out += layoutW; out += ",\"h\":"; out += SEG_ROWS; out += ",\"leds\":[";
    bool first = true;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        if (ledX[i] < 0) continue;
        if (!first) out += ",";
        first = false;
        out += "["; out += i; out += ","; out += ledX[i]; out += ","; out += ledY[i]; out += "]";
    }
    out += "]}";
    return out;
}

String ledsHex() {
    static const char hex[] = "0123456789abcdef";
    String out;
    out.reserve(LED_COUNT * 6 + 1);
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        uint32_t c = strip.getPixelColor(i);
        for (int sh = 20; sh >= 0; sh -= 4) out += hex[(c >> sh) & 0xF];
    }
    return out;
}

// Barva c s jasem k/255
uint32_t scaleColor(uint32_t c, uint8_t k) {
    return strip.Color(((c >> 16) & 0xFF) * k / 255, ((c >> 8) & 0xFF) * k / 255, (c & 0xFF) * k / 255);
}

// Duha, ktera se posouva zleva doprava pres cely napis
void fxRainbowWave() {
    for (uint16_t j = 0; j < 256; j++) {
        for (uint16_t i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, ledX[i] < 0 ? 0 : Wheel((ledX[i] * 256 / layoutW + 256 - j) & 255));
        }
        strip.show();
        WAIT(20);
    }
}

// Kometa: svetly sloupec s dohasinajicim ohonem jede pres napis
void fxComet() {
    const int tail = 8;
    for (int head = 0; head < layoutW + tail; head++) {
        for (uint16_t i = 0; i < LED_COUNT; i++) {
            int d = head - ledX[i];
            uint8_t k = (ledX[i] >= 0 && d >= 0 && d < tail) ? 255 - d * 255 / tail : 0;
            uint32_t c = settings.palette ? Wheel((head * 4) & 255) : settings.color;
            strip.setPixelColor(i, k ? scaleColor(c, k) : 0);
        }
        strip.show();
        WAIT(30);
    }
    WAIT(300);
}

// Trpyt: nahodne LED se rozsviti a pomalu pohasnou
void fxTwinkle() {
    static uint8_t lvl[LED_COUNT];
    static uint8_t col[LED_COUNT];
    for (int frame = 0; frame < 200; frame++) {
        for (int n = 0; n < 3; n++) {
            uint16_t i = random(0, LED_COUNT);
            lvl[i] = 255;
            col[i] = random(0, PALETTE_SIZE);
        }
        for (uint16_t i = 0; i < LED_COUNT; i++) {
            lvl[i] = lvl[i] * 7 / 8;
            strip.setPixelColor(i, lvl[i] > 4 ? scaleColor(paletteColor(col[i]), lvl[i]) : 0);
        }
        strip.show();
        WAIT(30);
    }
}

// Majaky: leva a prava polovina napisu se strida (cervena / modra)
void fxPolice() {
    uint32_t left  = settings.palette ? 0xFF0000 : settings.color;
    uint32_t right = settings.palette ? 0x0030FF : settings.color;
    for (int side = 0; side < 2; side++) {
        for (int flash = 0; flash < 3; flash++) {
            for (uint16_t i = 0; i < LED_COUNT; i++) {
                bool isLeft = ledX[i] >= 0 && ledX[i] < layoutW / 2;
                bool isRight = ledX[i] >= layoutW / 2;
                strip.setPixelColor(i, side == 0 ? (isLeft ? left : 0) : (isRight ? right : 0));
            }
            strip.show();
            WAIT(60);
            strip.clear();
            strip.show();
            WAIT(60);
        }
        WAIT(120);
    }
}

// Poradi = cisla efektu ulozena v nastaveni; nove efekty pridavat na konec.
const EffectInfo effects[] = {
    { "Celá show",           fxShow },
    { "Znaky postupně",      fxSegments },
    { "Jiskry",              fxSparkles },
    { "Sloupce",             fxColumns },
    { "EDU · IN · GO",       fxLetters },
    { "Plynulá duha",        fxRainbowCycle },
    { "Duha",                fxRainbow },
    { "Barevná písmena",     fxLettersRainbow },
    { "Vlna barvy",          fxColorWipe },
    { "Běžící světla",       fxTheaterChase },
    { "Běžící duha",         fxTheaterChaseRainbow },
    { "Dýchání",             fxBreathe },
    { "Jedna barva",         fxSolid },
    { "Vypnuto",             fxOff },
    { "Duhová vlna",         fxRainbowWave },
    { "Kometa",              fxComet },
    { "Třpyt",               fxTwinkle },
    { "Majáky",              fxPolice },
};
const uint8_t EFFECT_COUNT = sizeof(effects) / sizeof(effects[0]);
const uint8_t EFFECT_OFF = 13;   // index efektu "Vypnuto"

void setup() {
    Serial.begin(115200);
    loadSettings();
    buildLayout();

    strip.begin();
    applyBrightness();
    strip.show(); // Initialize all pixels to 'off'

    webSetup();
}

void loop() {
    effectRestart = false;
    static uint8_t lastEffect = 0xFF;
    if (settings.effect != lastEffect) { lastEffect = settings.effect; effectStart = millis(); }
    effects[settings.effect].run();
    if (effectRestart) {
        // efekt byl prerusen -> zacit novy z cisteho stavu
        strip.clear();
        strip.show();
    }
    webLoop();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
