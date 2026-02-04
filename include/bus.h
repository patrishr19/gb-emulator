#ifndef BUS_H
#define BUS_H

#include <setup.h>
typedef struct {
    // 64kb
    uint8_t memory[0x10000];
} Bus;


#endif