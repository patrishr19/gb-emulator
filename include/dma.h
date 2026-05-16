#pragma once
#include <setup.h>
#include <bus.h>
void dma_start(uint8_t start);
void dma_tick(Bus *bus);

bool dma_transfering();

