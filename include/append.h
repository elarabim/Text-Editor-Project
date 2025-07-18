#ifndef _APPEND_H_
#define _APPEND_H_

#include "main_text_editor.h"

extern terminal_configurations old_config;

void append_buffer(text_buffer* current_text_buff, char* c, int length_c);

void free_text_buffer(text_buffer* current_text_buffer);

#endif