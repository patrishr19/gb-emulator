# gb-emulator

Emulátor konzole Gameboy, který simuluje hardware gameboye a dokáže spouštět ROM soubory `.gb`. Program je primárně v Céčku. Tento projekt jsem si vybral z důvodu zlepšení se v jazyce C, pochopení jak fungují různé komponenty a jak se s nima pracuje.

## Dokumentace
Čerpám ze stránky [gbdev.io](https://gbdev.io/pandocs/About.html). 

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
- Používám knihovnu [Raylib](https://www.raylib.com/)
