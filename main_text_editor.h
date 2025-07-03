#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>  // for strerror
#ifndef MAIN_TEXT_EDITOR_H
#define MAIN_TEXT_EDITOR_H
#endif

/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)


typedef struct {
    struct termios old_settings ;
    struct winsize window_size; 
} terminal_configurations ;

typedef struct {
    char* text;
    int length;
} text_buffer ;



void draw_tildes(int ws_row, int col, text_buffer* tildes_buff);

void clear_screen(int ws_row, int col);

void kill(char* error_message);


char read_one_key();

void key_process();

struct winsize cursor_position();

struct winsize get_window_size();

void disable_raw_mode();

void enable_raw_mode(struct termios *settings);

void append_buffer(text_buffer* current_text_buff, char* c, int length_c);

void free_text_buffer(text_buffer* current_text_buffer);