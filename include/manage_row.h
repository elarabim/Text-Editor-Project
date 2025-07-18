#ifndef _MANAGE_ROW_H_
#define _MANAGE_ROW_H_

#include "syntax.h"

extern terminal_configurations old_config;

int cusror_x2render_x(plain_row *row,int cursor_x);

int rx_to_cx(plain_row* row, int rx);

void update_row(plain_row* row);

void insert_row(int idx, char* opening_line, ssize_t len);

void insert_char_in_a_row(plain_row* row, int idx, int c);

void delete_char_from_row(plain_row* row, int position);

void delete_row(int row_index);

void row_append_str(plain_row* erow,char *str,size_t len);

void free_row(plain_row *erow);

#endif