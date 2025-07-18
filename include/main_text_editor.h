#ifndef MAIN_TEXT_EDITOR_H
#define MAIN_TEXT_EDITOR_H

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




/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)

#define BEE_version "1.0"

#define BEE_TAB_STOP 8 //definir le nombre dont le tab arrêt s'il divise

#define QUIT_CONFIRMATION 3

#define HIGHLIGHT_NUMBERS (1<<0)

#define HIGHLIGHT_STRINGS (1<<1)


typedef struct {
    char* file_type ;
    char** file_match ; 
    char **keywords;
    char *comment_start_single_line;
    char *comment_start_multi_lines;
    char *comment_end_multi_lines;
    int flags ;
} Syntax ; 


enum highlights{
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_DIGITS,
    HL_MATCH
};

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
    unsigned char *highlight;
    int idx ;
    int hl_open_comment ; 
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
    int dirty;
    Syntax* syntax ; 
} terminal_configurations ;

typedef struct {
    char* text;
    int length;
} text_buffer ;


#endif
