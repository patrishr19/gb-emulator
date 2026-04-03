#ifndef PPU_SM_H
#define PPU_SM_H

#include <setup.h>

void ppu_mode_oam();
void ppu_mode_xfer(Bus *bus);
void ppu_mode_vblank(Bus *bus);
void ppu_mode_hblank(Bus *bus);

void increment_ly(Bus *bus);
#endif