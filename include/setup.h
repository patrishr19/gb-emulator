/**
 * @file setup.h
 * @brief SETUP
 * */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIT_SET(var, bit, val) ((val) ? ((var) |= (1 << (bit))) : ((var) &= ~(1 << (bit))))
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
