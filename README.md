# led-logo-firmware

Firmware pro RGB LED logo Eduingo (LOLIN C3 mini / ESP32-C3, 217× WS2812 na pinu 4).

## Ovládání přes WiFi

1. Po zapnutí logo vytvoří WiFi síť **Eduingo-Logo**, heslo **eduingo123**.
2. Připoj se telefonem; většina telefonů sama otevře ovládací stránku.
   Jinak otevři v prohlížeči **http://192.168.4.1**.
3. Na stránce jde přepínat efekt, jas, rychlost (100 % = původní tempo),
   hlavní barvu a jestli efekty používají pestrou paletu, nebo jen zvolenou barvu.

### Domácí WiFi a logo.local

V sekci **Domácí WiFi** na stránce klepni na *Vyhledat sítě*, vyber svou síť
(jen 2,4 GHz) a zadej heslo. Logo se k ní připojí a ovládání pak najdeš
v domácí síti na **http://logo.local** (nebo na IP adrese, kterou stránka ukáže).
Vlastní síť Eduingo-Logo zůstává zapnutá jako záloha, takže se k logu dostaneš
i při špatném hesle nebo mimo domov. Síť jde kdykoli změnit nebo zapomenout.

Nastavení se ukládá do flash (asi 3 s po poslední změně), takže po vypnutí
a zapnutí logo pokračuje stejným efektem.

Název sítě a heslo se mění v `src/web_ui.cpp` (`AP_SSID`, `AP_PASSWORD`, `HOSTNAME`).
Ovládací stránka je v `include/index_html.h`.
Nový efekt přidáš jako funkci `fx...()` v `src/main.cpp` a zapíšeš ji na konec
tabulky `effects[]`; místo `delay()` v ní používej `WAIT(ms)`, aby šlo efekt
z webu kdykoli přerušit.
