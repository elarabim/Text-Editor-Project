#ifndef _MANAGE_FILE_H_
#define _MANAGE_FILE_H_

#define _GNU_SOURCE       // Enables getline on GNU/Linux

#include "display.h"

extern terminal_configurations old_config;

char* rows_to_str(int* str_len);

void open_editor(char* filename);

void save_edited();

#endif
