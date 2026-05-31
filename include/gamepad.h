/**
 * @file gamepad.h
 * @brief The gamepad states, buttons and handlers
 * */
#pragma once

#include <setup.h>
/**
 * @brief Strcut of all input button states on the gamepad
 * */
typedef struct {
    bool start;
    bool select;
    bool a;
    bool b;
    bool up;
    bool down;
    bool left;
    bool right;
} gamepad_state;

void gamepad_init();
bool gamepad_button_sel();
bool gamepad_dir_sel();

/**
 * @brief Specifies input type
 * @param value the byte written to the gamepad register
 * */
void gamepad_set_sel(uint8_t value);

gamepad_state *gamepad_get_state();
uint8_t gamepad_get_output();
