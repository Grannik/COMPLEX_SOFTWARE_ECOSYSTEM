#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
//#include <libmsgtermcolor.h>
#include "libtermcolor/libtermcolor.h"

#define MIN_WIDTH 84
#define MIN_HEIGHT 21

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
void print_subtitle_left(const char *title, int row, int left_margin);

void universal_scroll(int module_id, int direction);
int universal_get_current_line(int module_id);
void universal_set_current_line(int module_id, int line);
int universal_get_block_size(int module_id);
void universal_set_block_size(int module_id, int size);
int universal_get_total(int module_id);

typedef const char** (*GetCharFunc)(char c);
void print_pseudographic_time(int hours, int mins, int secs,
                              GetCharFunc get_char_func,
                              int font_rows, int start_row, int start_col);

typedef enum {
    DISPLAY_LINES,
    DISPLAY_FUNCS
} display_mode_t;

void display_from_line_mode(int start_line, display_mode_t mode, void *content_ptr, int content_size);

#endif
