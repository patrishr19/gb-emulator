# gb-emulator

Emulator of the GameBoy console, that simulates it's hardware and is able to run ROM files with the `.gb` file extension. Program is developed primarily in C. I picked up this project due to the fact that i wanted to learn about low level stuff, how different components work and mainly enhance my C knowledge.

## Documentation
I'm currently taking inspire from the [pandocs](https://gbdev.io/pandocs/About.html).

## Goal
The final product should be a executable file for Linux and Windows and maybe some linked libraries.

## Ideas for the future
- Add a navigation bar on top that would bring easy access to the settings, saving and restarting
- Saving and loading states
- Xinput controller support
- Game speed modification (0.5, 1, 2)
- Settings
	- Keybind changes
	- Preferences

## Used libraries
- I'm using the [Raylib](https://www.raylib.com/) library.

## Installation guide

```bash
# 1. Clone the repository
git clone https://github.com/patrishr19/gb-emulator.git
cd gb-emulator

# 2. Compile it
make

# 3. Run your ROM
./emulator your/rom.gb
# Windows:
.\emulator.exe your\rom.gb
```

#####  Requirements
- C compiler (like gcc)
- Make

##### Linux (Debian)
```bash
sudo apt update && sudo apt install build-essential gcc make
```

##### Linux (Fedora)
```bash
sudo dnf groupinstall "Development Tools"
```

##### Linux (Arch)
```bash
sudo pacman -S base-devel
```

##### Windows 11 (not tested)
- Download and install [MSYS2](https://www.msys2.org/)
- Open the MSYS2 UCRT64 terminal
- Install the compiler
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc make
```
- Add `C:\msys64\ucrt64\bin` to the PATH (System environment variables)


#### Compiling
```bash
make
```
Rebuild
```bash
make clean
make
```

#### Note
- I do not provide any ROM files