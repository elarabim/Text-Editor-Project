#ifndef _SEARCH_H_
#define _SEARCH_H_

#include "display.h"

extern terminal_configurations old_config;

void search_query_callback(char *query, int key);

void search_query();

#endif