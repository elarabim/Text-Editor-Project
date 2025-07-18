#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "manage_row.h"
#include "append.h"
#include "terminal_config.h"


extern terminal_configurations old_config;

void draw_rows(int ws_row, int ws_col, text_buffer* tildes_buff);

void draw_status_bar(text_buffer *status_buff);

void draw_message_bar(text_buffer *message_buff);

void clear_screen(int ws_row, int ws_col);

void set_status_message(const char *format, ...);

void scroll();

char* Prompt(char* pr, void (*callback)(char* ,int));

#endif