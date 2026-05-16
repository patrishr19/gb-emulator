#pragma once

#include <setup.h>
#include <bus.h>
void ppu_mode_oam();
void ppu_mode_xfer(Bus *bus);
void ppu_mode_vblank(Bus *bus);
void ppu_mode_hblank(Bus *bus);

void increment_ly(Bus *bus);
