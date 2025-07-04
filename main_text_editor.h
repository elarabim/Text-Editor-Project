#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <ctype.h>
#include <string.h>  // for strerror
#ifndef MAIN_TEXT_EDITOR_H
#define MAIN_TEXT_EDITOR_H
#endif

/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)

#define BEE_version "1.0"

typedef struct{
    int row_size;
    char *row_data;
} plain_row;
typedef struct {
    int cursor_x;
    int cursor_y;
    int nrows;
    struct termios old_settings ;
    struct winsize window_size;
    plain_row editor_row; 
} terminal_configurations ;

typedef struct {
    char* text;
    int length;
} text_buffer ;



void OpenEditor();
void draw_tildes(int ws_row, int col, text_buffer* tildes_buff);

void clear_screen(int ws_row, int col);

void kill(char* error_message);

int read_one_key();

void key_process();

struct winsize cursor_position();

struct winsize get_window_size();

void disable_raw_mode();

void enable_raw_mode(struct termios *settings);

void append_buffer(text_buffer* current_text_buff, char* c, int length_c);

void free_text_buffer(text_buffer* current_text_buffer);

void move_cursor(int direction);
