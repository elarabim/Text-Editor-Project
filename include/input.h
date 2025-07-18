#ifndef _INPUT_H_
#define _INPUT_H_


#include "terminal_config.h"
#include "manage_file.h"
#include "edit.h"
#include "search.h"

extern terminal_configurations old_config;

void valid_row(plain_row **row, terminal_configurations old_config);

void move_cursor(int direction);

void key_process();

#endif