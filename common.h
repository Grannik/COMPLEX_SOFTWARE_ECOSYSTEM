#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

extern const char **current_content;
extern int current_content_size;
extern int content_block_size;
extern int content_current_line;

void display_from_line(int start_line);
void screen_state(int flag);
void term_mode(int flag);
int wait_switch(void);
void wait_exit(void);
void run_animated_module(int (*module_func)(void));
void draw_exact_frame(void);
int get_terminal_height(void);
void common_scroll(int direction, int *current_line, int block_size, int total);

void module_0_scroll_up(void);
void module_0_scroll_down(void);
int module_0_get_current_line(void);
void module_0_set_current_line(int line);

void module_5_scroll_up(void);
void module_5_scroll_down(void);
int module_5_get_current_line(void);
void module_5_set_current_line(int line);

#endif
