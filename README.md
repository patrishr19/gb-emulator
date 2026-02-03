# gb-emulator

Emulátor konzole Gameboy, který bude simulovat hardware gameboye a  dokáže spouštět ROM soubory `.gb`. Program bude v Céčku. Vybral jsem si tento projekt kvůli tomu abych se zlepšil v jazyce C a pochopil jak fungují různé komponenty a jak se s nima pracuje. Také se naučím jak funguje memory mapping.

## Dokumentace
Budu čerpat ze stránky [gbdev.io](https://gbdev.io/pandocs/About.html). 

## Cíl
Výsledkem by měl být `.exe` soubor a možná nějaké `.dll` soubory

## Do budoucna
- Přidat navigační bar nahoře který by obsahoval drop down menu nastavení.
- Save State/Load state
- Xinput controller support
- Úprava rychlosti hry (0.5, 1, 2)
- Nastavení
	- Úprava keybindů
	- Nastavení zvuku atd.

## Použité knihovny
- Použil bych knihovnu [Raylib](https://www.raylib.com/)
