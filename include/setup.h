#ifndef SETUP_H
#define SETUP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIT_SET(var, bit, val) ((val) ? ((var) |= (1 << (bit))) : ((var) &= ~(1 << (bit))))

#endif