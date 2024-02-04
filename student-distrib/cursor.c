/* all information below are adopted from https://https://wiki.osdev.org/Text_Mode_Cursor */
#include "cursor.h"
#include "lib.h"
/*
 * enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
 *   Description: Enabling the cursor allows us to set the start and end scanlines
 *   Inputs: uint8_t cursor_start, uint8_t cursor_end
 *   Outputs: none
 *   Side effects: Enabling the cursor
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x0A, 0x3D4);
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
	outb(0x0B, 0x3D4);
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}

/*
 * disable_cursor()
 *   Description: 
 *
 *   Inputs: none
 *   Outputs: none
 *   Side effects: 
 */
void disable_cursor()
{
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}

/*
 * update_cursor(int x, int y)
 *   Description: 
 *
 *   Inputs: int x, int y
 *   Outputs: none
 *   Side effects: 
 */
void update_cursor(int x, int y)
{
	uint16_t pos = y * 80 + x;
 
	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

/*
 * get_cursor_position(void)
 *   Description: 
 *
 *   Inputs: none
 *   Outputs: pos
 *   Side effects: 
 */
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(0x0F, 0x3D4);
    pos |= inb(0x3D5);
    outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}
