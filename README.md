# led-logo-firmware

Firmware pro RGB LED logo Eduingo (LOLIN C3 mini / ESP32-C3, 217× WS2812 na pinu 4).

## Ovládání přes WiFi

1. Po zapnutí logo vytvoří WiFi síť **Eduingo-Logo**, heslo **eduingo123**.
2. Připoj se telefonem; většina telefonů sama otevře ovládací stránku.
   Jinak otevři v prohlížeči **http://192.168.4.1**.
3. Na stránce jde přepínat efekt, jas, rychlost (100 % = původní tempo),
   hlavní barvu a jestli efekty používají pestrou paletu, nebo jen zvolenou barvu.

Nastavení se ukládá do flash (asi 3 s po poslední změně), takže po vypnutí
a zapnutí logo pokračuje stejným efektem.

Název sítě a heslo se mění v `src/web_ui.cpp` (`AP_SSID`, `AP_PASSWORD`).
Nový efekt přidáš jako funkci `fx...()` v `src/main.cpp` a zapíšeš ji na konec
tabulky `effects[]`; místo `delay()` v ní používej `WAIT(ms)`, aby šlo efekt
z webu kdykoli přerušit.
