/**
 * @file emulator.h
 * @brief The main system components in a struct
 * */
#pragma once

#include <setup.h>
#include <cpu.h>
#include <bus.h>
/**
 * @brief the main Gameboy struct
 * */
typedef struct{
    CPU cpu;
    Bus bus;
} Gameboy;

