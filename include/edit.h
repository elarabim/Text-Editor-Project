#ifndef _EDIT_H_
#define _EDIT_H_

#include "manage_row.h"

extern terminal_configurations old_config;

void insert_char_in_the_editor(int c);

void new_line_insert();

void delete_char();

#endif