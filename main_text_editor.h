#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#include <ctype.h>
#include <string.h> 
#include <time.h>
#include <stdarg.h>

#ifndef MAIN_TEXT_EDITOR_H
#define MAIN_TEXT_EDITOR_H
#endif

/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)

#define BEE_version "1.0"

#define BEE_TAB_STOP 8 //definir le nombre dont le tab arrêt s'il divise

enum editorKey {
  ARROW_LEFT = 256, // in GCC and Clang chars are signed and coded in a byte, so 128 is an out of range
                    // which will help us to differentiate from regular chars (previously 'w')
                    // However since windows uses the MSVC compiler that considers chars unsigned we chose 256 just to be safe (1 unsigned byte -> 255)
  BACKSPACE = 127,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DELETE_KEY
};



typedef struct{
    int row_size;
    int ren_size;
    char *row_data;
    char *render;//a string to handle tab for now,as the size of tab when applied is 8,when it can be written in 1 byte

} plain_row;


typedef struct {
    int cursor_x;
    int cursor_y;
    int render_x; //cursor when considering tabs
    int nrows; // number of rows in the file
    struct termios old_settings ;
    struct winsize window_size;
    int row_offset; // which line of the file is shown at the top of the window
    int column_offset;
    plain_row* editor_row; // a row from the file
    char* file_name;
    char status_message[64];
    time_t status_message_time;
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

void insert_row(char* opening_line, ssize_t len) ;

void scroll();

void valid_row(plain_row **row, terminal_configurations old_config);

void update_row(plain_row* row);

int cusror_x2render_x(plain_row *row,int cursor_x);

void intialize_editor();

void open_editor(char* filename);

void draw_message_bar(text_buffer *message_buff);

void draw_status_bar(text_buffer *status_buff);

void set_status_message(const char *format, ...);
