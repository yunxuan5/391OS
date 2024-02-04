#include    "terminal.h"
#include    "keyboard.h"
#include    "lib.h"
#include    "types.h"
#include    "cursor.h"
#include    "system_calls.h"
#include    "paging.h"
#include    "scheduler.h"

#define SUCCESS         0
#define FAIL            -1
#define VIDEO_MEM      0xB8000

int i, j;
// terminal_t terminal;

int curr_term_index = 0;

/*
 * terminal_init()
 *   Description: initial our terminal structure
 *   Inputs: none
 *   Outputs: none
 *   Side effects: initial cursor coordinate and empty the buffer
 */
void terminal_init() {
    for(i = 0; i < 3; i++){
        terminals[i].cursor_x = 0;
        terminals[i].cursor_y = 0;
        terminals[i].enter_flag = 0;
        terminals[i].buf_index = 0;
        terminals[i].ID = i;
        // terminals[i].video_page = VIDEO_MEM + (i+1) * 0x1000;
        terminals[i].shell_on = 0;
        if(i == 0){
            terminals[i].video_page = 0xB9000;
        }
        if(i == 1){
            terminals[i].video_page = 0xBA000;
        }
        if(i == 2){
            terminals[i].video_page = 0xBB000;
        }
        // empty the line buffer
        for(j = 0; j < 128; j++) {
            terminals[i].line_buffer[j] = '\0';
        }
        set_pte((terminals[i].video_page) >> 12, 1);
    }

    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );

    // terminal.cursor_x = 0;
    // terminal.cursor_y = 0;
    // terminal.enter_flag = 0;
    // terminal.buf_index = 0;

    // empty the line buffer
    // for(i = 0; i < 128; i++) {
    //     terminal.line_buffer[i] = '\0';
    // }
    clear();
    printf("try to open first shell\n");
    // execute((const uint8_t*)"shell");
    enable_cursor(0, 14);
    update_cursor(terminals[0].cursor_x, terminals[0].cursor_y);
}

/*
 * terminal_read (int32_t fd, void* buf, int32_t nbytes)
 *   Description: read should return data from one line that has been terminated by pressing
 *                Enter, or as much as fits in the buffer from one such line. 
 *                The line returned should include the line feed character.
 *   Inputs: int32_t fd, void* buf, int32_t nbytes
 *   Outputs: bytes we read
 *   Side effects: initial cursor coordinate and empty the buffer
 */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes) {
    int count = 0;
    if (buf == NULL) return FAIL;

    // while(terminal.enter_flag == 0){}   // wait until enter is pressed
    while(curr_term()->enter_flag == 0){}   // wait until enter is pressed

    cli();
    // clear the buf we need to read into
    for(i = 0; i < 128; i++) {
        ((char*)buf)[i] = '\0';
    }
    
    // iterate elements in line buffer
    for(i = 0; i < nbytes && i < curr_term()->buf_index; i++){
        ((char*)buf)[i] = curr_term()->line_buffer[i];
        // if we reach the end of postion and the char is not next line char, we change that one to '\n'
        // last char of buffer should always be '\n'
        if((i == nbytes-1) && (((char*)buf)[i] != '\n')){
            ((char*)buf)[i] = '\n';
        }
        count++;
    }

    // done reading, clear the line buffer
    for(i = 0; i < 128; i++){
        curr_term()->line_buffer[i] = '\0';
    }
    curr_term()->enter_flag = 0;
    curr_term()->buf_index = 0;
    sti();

    return count;
}

/*
 * terminal_write (int32_t fd, const void* buf, int32_t nbytes)
 *   Description:  all data should of the buf be displayed to the screen immediately
 *   Inputs: int32_t fd, const void* buf, int32_t nbytes
 *   Outputs: num of bytes we write
 */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes) {
    int i;
    int count = 0;
    if(buf == NULL) return FAIL;
    cli();
    // iterate elements in buf and display on screen
    for(i = 0; i < nbytes; i++) {
        if(((char*)buf)[i] != '\0'){
            putc(((char*)buf)[i]);
            count++;
        } 
    }
    sti();
    return count;
}


/*
 * terminal_open (const uint8_t* filename)
 *   Description:  system call provides access to the file system
 *   Inputs: const uint8_t* filename
 *   Outputs: successful or fail
 */
int32_t terminal_open (const uint8_t* filename) {
    return SUCCESS;
}

/*
 * int32_t terminal_close (int32_t fd)
 *   Description:  close system call closes the specified file descriptor and makes it available for return from later calls to open
 *   Inputs: int32_t fd
 *   Outputs: successful or fail
 */
int32_t terminal_close (int32_t fd) {
    return SUCCESS;
}

/*
 * write_backspace(char input)
 *   Description:  handle backspace in terminal, we clear current char and decrease the index
 *   Inputs: char input
 *   Outputs: none
 */
void write_backspace(char input) {
    if(curr_term()->buf_index != 0){
        curr_term()->buf_index--;
        curr_term()->line_buffer[curr_term()->buf_index] = input;
    }
}

/*
 * write_enter(char input)
 *   Description:  handle enter in terminal, we set last char to '\n'
 *   Inputs: char input
 *   Outputs: 
 */
void write_enter(char input) {
    curr_term()->enter_flag = 1;
    if(curr_term()->buf_index >= 128){
        curr_term()->line_buffer[127] = input;
    } else {
        curr_term()->line_buffer[curr_term()->buf_index] = input;
        curr_term()->buf_index++;
    }

}

/*
 * write_char(char input)
 *   Description:  handle regular key in terminal
 *   Inputs: char input
 *   Outputs: 
 */
void write_char(char input) {
    if (curr_term()->buf_index < 127)
    {
        curr_term()->line_buffer[curr_term()->buf_index] = input;
        curr_term()->buf_index++;
    }
    
}
/*
 * terminal_t* curr_term()
 *   Description: use for other to get access of the current viewing terminal
 *   Inputs: none
 *   Outputs: viewing terminal
 *   Side effects: 
 */
terminal_t* curr_term(){
    return &terminals[curr_term_index];
}

/*
 * void terminal_switch(int index)
 *   Description: Switch terminal when we press alt and F1/F2/F3
 *   We need to restore our mem to video mem
 *   Inputs: int index
 *   Outputs: none
 *   Side effects: update cursor of new terminal
 */
void terminal_switch(int index){
    if (index == curr_term_index){
        return;
    }
    //cli();
    terminal_t* cur_terminal_ptr = &terminals[curr_term_index];
    terminal_t* new_terminal = &terminals[index];
    // update cursor for the new terminal
    update_cursor(new_terminal->cursor_x, new_terminal->cursor_y);
    // first update the video memory paging
    vidmap_switch(curr_term_index);
    // restore video memory to current viewing mem page and put new terminal's video page to video mem
    memcpy((void*) cur_terminal_ptr->video_page, (void*) VIDEO_MEM, 0x1000);
    memcpy((void*) VIDEO_MEM, (void*)  new_terminal->video_page, 0x1000);
    curr_term_index = index;
    // restore the video memory paging
    vidmap_switch(curr_index);
    //sti();

}

void vidmap_switch(int index){
    // if the current running terminal is the one shows on the screen
    if(index == curr_term_index){
        page_table[VIDEO_MEM >> 12].page_address = VIDEO_MEM >> 12;
        page_table_vidmap[VIDEO_MEM >> 12].page_address = VIDEO_MEM >> 12;
        page_table_vidmap[VIDEO_MEM >> 12].present = 1;
    // if the current running terminal is not the one shows on the screen
    } else {
        page_table[VIDEO_MEM >> 12].page_address = terminals[index].video_page >> 12;
        page_table_vidmap[VIDEO_MEM >> 12].page_address = terminals[index].video_page >> 12;
        page_table_vidmap[VIDEO_MEM >> 12].present = 0;
    }
    //flush tlb
    asm volatile (
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax"    //clobbers eax
    );

    return;
}
