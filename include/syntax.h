#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "main_text_editor.h"

extern terminal_configurations old_config;

extern char *HL_C_extensions[];
extern char *HL_C_KEYWORDS[];
extern Syntax HL_Database[];

#define HL_DATABASE_ENTRIES (sizeof(HL_Database) / sizeof(HL_Database[0]))

int is_separator(int c);

void update_syntax(plain_row *row);

int color_syntax(int highlight);

void syntax_highlight();

#endif