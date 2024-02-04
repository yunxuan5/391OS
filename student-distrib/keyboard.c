#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "cursor.h"
#include "system_calls.h"
#include "system_calls_linkage.h"

#define NUM_KEYS           0x3B
#define IRQ_NUM            1

#define BACKSPACE_ON       0x0E
#define LEFT_CONTROL_ON    0x1D
#define LEFT_SHIFT_ON      0x2A
#define RIGHT_SHIFT_ON     0x36
#define CAPSLOCK_ON        0x3A
#define ENTER_ON           0x1C
#define KEY_L              0x26
#define KEY_C              0x2E
#define ALT_ON             0x38
#define F1                 0x3B
#define F2                 0x3C
#define F3                 0x3D

#define BACKSPACE_OFF      0x8E
#define LEFT_CONTROL_OFF   0x9D
#define LEFT_SHIFT_OFF     0xAA
#define RIGHT_SHIFT_OFF    0xB6
#define CAPSLOCK_OFF       0xBA
#define ALT_OFF             0xB8


uint8_t control_pressed;
uint8_t shift_pressed;
uint8_t alt_pressed;
uint8_t capslock_pressed;

int buf_count = 0; 
// buf_count = terminals[cur]
int i;
int j;

static char keyboard_dict[NUM_KEYS][2] = {
                    {'\0', '\0'}, {'\0', '\0'},{'1', '!'},{'2', '@'},
                    {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'},
                    {'7', '&'}, {'8', '*'}, {'9', '('}, {'0', ')'},
                    {'-', '_'}, {'=', '+'}, {'\b', '\b'}, {'\t', '\t'},
                    {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'},
                    {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, {'i', 'I'},
                    {'o', 'O'}, {'p', 'P'}, {'[', '{'}, {']', '}'},
                    {'\n', '\n'}, {'\0', '\0'}, {'a', 'A'}, {'s', 'S'},
                    {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'},
                    {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'},
                    {'\'', '"'}, {'`', '~'}, {'\0', '\0'}, {'\\', '|'},
                    {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'},
                    {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'},
                    {'.', '>'},  {'/', '?'}, {'\0', '\0'}, {'\0', '\0'},
                    {'\0', '\0'}, {' ', ' '}, {'\0', '\0'}
                    };

/*
 * keyboard_init()
 *   Description: enable to accept interrupt from keyboard
 *   Inputs: none
 *   Outputs: none
 *   Side effects: enable to accept interrupt from keyboard
 */
void keyboard_init(){
    enable_irq(IRQ_NUM);
}

/*
 * keyboard_irq_handler()
 *   Description: handle the interrupt from keyboard. We first reads a byte
 *                from the port and match it with our keyboard_dict. And then
 *                we print the value to the screen
 *   Inputs: none
 *   Outputs: none
 *   Side effects: we only care about lower case char and number
 */
void keyboard_irq_handler(){
    // still need irq_num to send the signal
    send_eoi(IRQ_NUM);
    uint8_t scancode;
    scancode = inb(0x60) & 0xFF;   //get scancode from port 0x60

    switch(scancode){
        case LEFT_CONTROL_ON:
            control_pressed = 1;
            break;

        case LEFT_SHIFT_ON:
        case RIGHT_SHIFT_ON:
            shift_pressed = 1;
            break;

        case CAPSLOCK_ON:
            capslock_pressed = 1 - capslock_pressed;
            break;

        case LEFT_CONTROL_OFF:
            control_pressed = 0;
            break;

        case LEFT_SHIFT_OFF:
        case RIGHT_SHIFT_OFF:
            shift_pressed = 0;
            break;
        case ALT_ON:
            alt_pressed = 1;
            break;
        case ALT_OFF:
            alt_pressed = 1;
            break;

        case F1:
            if(alt_pressed){
                terminal_switch(0);
            }
            break;

        case F2:
            if(alt_pressed){
                terminal_switch(1);
            }
            break;

        case F3:
            if(alt_pressed){
                terminal_switch(2);
            }
            break;
        // otherwise, the scancode is not function key and we need to display on the screen
        default:  
            regular_press(scancode);
            break;
    }

}

/*
 * regular_press(uint8_t scancode)
 *   Description: handle the regular input from the scancode that need 
 *                to show on the screen(letter, number, backspace, tap and enter)
 *   Inputs: uint8_t scancode
 *   Outputs: none
 *   Side effects: we only care about input that need to show on the screen.
 */
void regular_press(uint8_t scancode) {
    char char_to_print;
    // out of range, use to not crush kernel
    // if (scancode >= NUM_KEYS){
    //     return;
    // }
    if (scancode >= 0x3B){
        return;
    }

    // if (alt_pressed && (scancode == F1 || scancode == F2 || scancode == F3)){
    //     switch (scancode)
    //     {
    //         case F1:
    //             terminal_switch(0);
    //             break;
            
    //         case F2:
    //             terminal_switch(1);
    //             break;

    //         case F3:
    //             terminal_switch(2);
    //             break;

    //         default:
    //             break;
    //     }
                
    // }

    // case when control+l should clean the screen and reset the cursor to the top left of the screen
    if (control_pressed && scancode == KEY_L) {
        // clean screen
        clear();
        // reset cursor coordinate
        curr_term()->cursor_x = 0;
        curr_term()->cursor_y = 0;
        update_cursor(curr_term()->cursor_x, curr_term()->cursor_y);
        return;
    }



    if (control_pressed && scancode == KEY_C) { // handle control + c
        printf("\n");
        system_calls(halt(0x00));
        return;
    }

    // case when capslock and shift are on
    else if (capslock_pressed && shift_pressed) {
        if ((scancode >= 0x10 && scancode <= 0x19) || (scancode >= 0x1E && scancode <= 0x26) || (scancode >= 0x2C && scancode <= 0x32)) {
            char_to_print = keyboard_dict[scancode][0];
        } else {
            char_to_print = keyboard_dict[scancode][1];
        }
        
    }

    // case when only capslock is on
    else if (capslock_pressed) {
        // if the key is letter, make it capital
        if ((scancode >= 0x10 && scancode <= 0x19) || (scancode >= 0x1E && scancode <= 0x26) || (scancode >= 0x2C && scancode <= 0x32)) {
            char_to_print = keyboard_dict[scancode][1];
        } else {

        // otherwise, we just print the normal one
            char_to_print = keyboard_dict[scancode][0];
        }
    }
    
    // case when shift is pressed
    else if (shift_pressed) {
        char_to_print = keyboard_dict[scancode][1];
    }

    // case when no function key is pressed
    else if (!capslock_pressed && !shift_pressed) {
        char_to_print = keyboard_dict[scancode][0];
    } 

    // we need to handel the case of "enter", "backspace", "tab", and regular key
    // enter is pressed
    if (char_to_print == '\n')
    {
        enter_pressed();
    // backspace is pressed
    } else if (char_to_print == '\b') {
        if(buf_count > 0) backspace_pressed();
    }
    // tab is pressed 
    else if (char_to_print == '\t') {
        tab_pressed();
    }
    // regular key is pressed 
    else {
        // if we have 128 keys already, we won't read next key
        if (buf_count >= 127 && keyboard_dict[scancode][0] != '\n') return;

        // otherwise, put the key on the screen and also fill in buffer in the terminal
        putc(char_to_print);
        buf_count++;;
        write_char(char_to_print);
    }
}

/*
 * backspace_pressed()
 *   Description: handle when backspace is pressed, we should decrease our counter
 *                and write it into terminal buffer
 *   Inputs: none
 *   Outputs: none
 *   Side effects: we write an empty char into buffer
 */
void backspace_pressed() {
    putc('\b');
    buf_count--;
    write_backspace('\0');
}

/*
 * enter_pressed()
 *   Description: handle when enter is pressed, we should clear our counter
 *                and write it into terminal buffer
 *   Inputs: none
 *   Outputs: none
 *   Side effects: we write an next line char into buffer
 */
void enter_pressed() {
    putc('\n');
    buf_count = 0;
    write_enter('\n');
}

/*
 * tab_pressed()
 *   Description: handle when tap is pressed, we should increase our counter
 *                by 4 and write 4 spaces into terminal buffer
 *   Inputs: none
 *   Outputs: none
 *   Side effects: we write an 4 spaces char into buffer
 */
void tab_pressed() {
    for (i = 0; i < 4; i++) {
        putc(' ');
        buf_count++;
        write_char(' ');
    }
}


