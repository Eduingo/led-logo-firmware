#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    4    // datovy pin pasku
#define LED_COUNT  217  // pocet LED na pasku
// Pozor na odber: 217 LED na plnou bilou muze teoreticky brat az ~13 A.
// Pokud zdroj nestaci (bliká, resetuje se ESP), sniz jas napr. na 80-120.
#define BRIGHTNESS 255

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

void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

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
        delay(wait);
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
        delay(wait);
    }
}

// Zhasne cely znak sloupec po sloupci
void glowdownSegment(uint8_t segment, uint8_t wait) {
    glowupSegment(segment, 0, wait);
}

void setup() {
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    strip.show(); // Initialize all pixels to 'off'
}

void loop() {
    const uint32_t colors[] = {
        strip.Color(255, 0, 0),   // Red
        strip.Color(0, 255, 0),   // Green
        strip.Color(0, 0, 255),   // Blue
        strip.Color(255, 255, 0), // Yellow
        strip.Color(0, 255, 255), // Cyan
        strip.Color(255, 0, 255), // Magenta
        strip.Color(255, 255, 255) // White
    };

    uint32_t c = strip.Color(100, 10, 130); // Purple
    // Glowup all segments with different colors
    for (size_t s = 0; s < sizeof(segments) / sizeof(segments[0]); s++)
    {
        glowupSegment(s, colors[s % (sizeof(colors) / sizeof(colors[0]))], 10);
    }

    delay(2000);

    // Glowdown all segments
    for (size_t s = 0; s < sizeof(segments) / sizeof(segments[0]); s++)
    {
        glowdownSegment(s, 10);
    }
    // LED c. 217 neni v mapovani, takze by po rainbowCycle zustala svitit -> zhasnout vse
    strip.clear();
    strip.show();

    // make random shines
    for (size_t i = 0; i < 25; i++)
    {
        uint16_t pixels[12];
        for (size_t k = 0; k < 12; k++)
        {
            pixels[k] = random(0, strip.numPixels());
            uint32_t color = colors[random(0, sizeof(colors) / sizeof(colors[0]))];
            strip.setPixelColor(pixels[k], color);
        }
        strip.show();
        delay(30);
        
        for (size_t k = 0; k < 12; k++)
        {
            strip.setPixelColor(pixels[k], 0); // Off
        }
        strip.show();
    }

    // rozsviceni sloupec po sloupci
    for (size_t col = 0; col < SEG_COLS; col++)
    {
        glowupColumn(col, c, 5);
    }

    delay(2000);

    // zhasinani sloupec po sloupci
    for (size_t col = 0; col < SEG_COLS; col++)
    {
        glowdownColumn(col, 5);
    }

    // colorWipe(strip.Color(100, 10, 130), 5); // Purple
    delay(1000);

    // Rozsvit pouze segmenty 1,2,3
    glowupSegment(1, strip.Color(255, 0, 255),50); // E
    glowupSegment(2, strip.Color(255, 0, 255), 50); // D
    glowupSegment(3, strip.Color(255, 0, 255), 50); // U
    delay(300);
    glowdownSegment(1, 50); // E
    glowdownSegment(2, 50); // D
    glowdownSegment(3, 50); // U
    glowupSegment(4, strip.Color(0, 255, 255), 50); // I
    glowupSegment(5, strip.Color(0, 255, 255), 50); // N
    delay(300);
    glowdownSegment(4, 50); // I
    glowdownSegment(5, 50); // N
    glowupSegment(6, strip.Color(0, 255, 0), 50); // G
    glowupSegment(7, strip.Color(0, 255, 0), 50); // O
    delay(300);
    glowupSegment(0, strip.Color(255, 255, 255), 50); // LOGO

    delay(100);
    glowdownSegment(6, 50); // G
    glowdownSegment(7, 50); // O

    // rozsvit vsechny znaky bile, pak duha
    for (size_t i = 1; i <= 7; i++)
    {
        glowupSegment(i, strip.Color(255, 255, 255), 20);
    }

    rainbowCycle(1);
    
    // colorWipe(strip.Color(255, 0, 0), 20); // Red
    // colorWipe(strip.Color(0, 255, 0), 20); // Green
    // colorWipe(strip.Color(0, 0, 255), 20); // Blue
    // //colorWipe(strip.Color(0, 0, 0, 255), 20); // White RGBW
    // // Send a theater pixel chase in...
    // theaterChase(strip.Color(127, 127, 127), 20); // White
    // theaterChase(strip.Color(127, 0, 0), 20); // Red
    // theaterChase(strip.Color(0, 0, 127), 20); // Blue

    // rainbow(20);
    // rainbowCycle(20);
    //theaterChaseRainbow(20);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
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
