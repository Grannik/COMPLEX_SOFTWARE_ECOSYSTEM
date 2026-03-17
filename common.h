#ifndef COMMON_H
#define COMMON_H

#include <ncursesw/ncurses.h>

typedef enum {
    MODULE_NCURSES,
    MODULE_ANSI
} ModuleType;

extern ModuleType current_mode;

extern int termheight;
extern int termwidth;
extern int current_content_size;
extern int content_block_size;
extern int content_current_line;

int terminal_control(int mode);
int terminal_check_size(int min_width, int min_height);
void display_content_with_scroll(WINDOW *win, int start_y, int start_x, int height, int width, const char **content, int total_lines, int *scroll_offset);
int handle_scroll_keys(int ch, int *scroll_offset, int total_lines, int visible_lines);

void common_scroll(int direction, int *current_line, int block_size, int total);
void universal_scroll(int module_id, int direction);
int universal_get_current_line(int module_id);
void universal_set_current_line(int module_id, int line);
int universal_get_block_size(int module_id);
void universal_set_block_size(int module_id, int size);
int universal_get_total(int module_id);
typedef enum {
    DISPLAY_LINES,
    DISPLAY_FUNCS
} display_mode_t;
 void display_from_line(int start_line, void *content, int content_size);
 void term_control_termios(int flag);
 int handle_common_input(char input_char, int *current_module);

typedef struct {
    int first_draw;
    void (*draw_frame)(void);
} Module;

void module_draw(Module modules[], int count, int index);

// универсальная функция для первой отрисовки отдельного модуля
void module_first_draw(int *first_draw, void (*draw_frame)(void));

#endif
