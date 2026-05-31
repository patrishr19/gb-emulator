/**
 * @file dma.h
 * @brief Direct memory access pipeline
 */
#pragma once
#include <setup.h>
#include <bus.h>
void dma_start(uint8_t start);

/**
 * @brief Steps the DMA pipeline, transfers data in cycles
 * */
void dma_tick(Bus *bus);
/**
 * @brief Checker if the dma is currently dma_transfering
 * @return false if DMA is idle
 * */
bool dma_transfering();

