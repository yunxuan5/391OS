#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include "types.h"

// enable keyboard to accept input
extern void keyboard_init();
// handle different function keys
extern void keyboard_irq_handler();
// handle regular key pressed
extern void regular_press(uint8_t scancode);
// handle backspace pressed
extern void backspace_pressed();
// handle enter pressed
extern void enter_pressed();
// handle tab pressed
extern void tab_pressed();

#endif
