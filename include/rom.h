/**
 * @file rom.h
 * @brief ROM file loading
 * */

#pragma once

#include <setup.h>
#include <bus.h>

bool LoadRom(Bus *bus, const char *filename);

const char *select_rom_dialog();

