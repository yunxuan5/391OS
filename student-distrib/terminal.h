#ifndef _TERMINAL_H
#define _TERMINAL_H
#include "types.h"

#define NUM_TERMS 3

typedef struct terminal_t {
    char    line_buffer[128];
    int     buf_index;
    int     cursor_x;
    int     cursor_y;
    volatile int enter_flag;
    uint32_t active_pid;
    uint32_t video_page;
    int shell_on;
    int ID;
} terminal_t;

terminal_t terminals[NUM_TERMS];
extern int curr_term_index;
void    terminal_init();
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t terminal_open (const uint8_t* filename);
int32_t terminal_close (int32_t fd);

void write_backspace(char input);
void write_enter(char input);
void write_char(char input);
extern terminal_t* curr_term();
void terminal_switch(int index);
void vidmap_switch(int index);

#endif
