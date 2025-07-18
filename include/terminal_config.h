#ifndef _TERMINAL_CONFIG_H_
#define _TERMINAL_CONFIG_H_

#include "display.h"

extern terminal_configurations old_config;

int read_one_key();

struct winsize get_window_size();

void kill(char* error_message);

void disable_raw_mode();

void enable_raw_mode(struct termios *settings);

struct winsize cursor_position();

#endif