#pragma once

#ifndef LIBTECOLOR_H
#define LIBTECOLOR_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "tc_file_types.h"
#include "tc_frame.h"

#define VOID 0
#define MAX_PAIRS 1024

typedef struct {
    short fg;
    short bg;
    short pair;
} ColorPairEntry;

int get_color_pair(int fg, int bg);
void color_init(void);
void color_info(void);
void reset_ansi(int flag);

#define ATTR_NONE   0
#define ATTR_BOLD   1
#define ATTR_UNDER  2

int is_file_category(const char *filename, unsigned int category_flags);

FileType get_file_type(const char *filename);

const char *get_file_type_name(FileType type);
int is_file_type(const char *filename, FileType type);
int get_text_module_file_color(const char *filename, int is_dir, char flag);
int get_analyzer_file_color(const char *filename, int is_dir);

#define OUTER_FRAME_WIDTH 84
#define FILE_LIST_FIXED_WIDTH 82

#define LEFT   0
#define CENTER 1
#define RIGHT  2

int get_char_width(wchar_t c);
int compute_wchar_width(const wchar_t *w, size_t len);
int calculate_visual_width(const char *src);
int convert_to_wchar(const char *src, wchar_t **wsrc_out, size_t *wlen_out);
int prepare_display_wstring(const char *src, int max_visual_width, wchar_t *dest, size_t dest_size, int add_suffix, const wchar_t *ellipsis, int visual_offset, int use_middle_ellipsis);

void generic_frame_ncurses(
WINDOW *win, int x, int y, int width, int height, int attr, int fg, int bg, int bgsingl, int title_align, int title_int, FrameStyleType style_type, const char *fmt, ...);
void generic_msg_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_msg_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...);

 void generic_frame_ansi(
 int mod, int x, int y, int width, int height, int attr, int fg, int bg, int bgsingl, int title_align, int title_int, FrameStyleType style_type, const char *fmt, ...);

void module_strip_ansi(int mod, int x, int y, int attr, int frame_fg, int frame_bg, int text_fg, char symbol, const char *module_name);
const char** get_pseudographic_char_3(char c);
void generic_abc_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);

const char** get_pseudographic_char_5(char c);
void generic_abc_5_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_5_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);

void generic_abc_7_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_7_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);

typedef struct Message {
    int x;
    const char *text;
} Message;

void group_generic_msg_ncurses(int y, int attr, int fg, int bg, Message *messages, int count);
void group_msg(int y, int attr, int fg, int bg, ...);
void var_generic_msg_ncurses(int y, int attr, int fg, int bg, ...);

void group_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

void group_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

void generic_abc_9_ansi(int color_mode, int x, int y, int attr, int fg, int bg, const char *fmt, ...);
void generic_abc_9_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);

attr_t convert_attr_to_ncurses(int attr);

void attr_ansi(int attr);
void mod_fg_ansi(int mod, int color);
void mod_bg_ansi(int mod, int color);
void x_y_ansi(int x, int y);
void reset_ansi(int flag);

void var_abc_9_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

void module_strip_ncurses(WINDOW *win, int x, int y, int attr, int frame_fg, int frame_bg, int text_fg, char symbol, const char *module_name);

void generic_cycle_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...);

void cycle_frame_ncurses(
    WINDOW *win,
    int x, int y,
    int width, int height,
    int attr,
    int fg,
    int bg,
    int bgsingl,
    int title_align,
    int title_int,
    FrameStyleType style_type,
    const char *fmt, ...);

void txt_curs(const char* fmt);
void add_ellipsis(wchar_t *dest, size_t *dest_idx, size_t dest_size, const wchar_t *ellipsis, size_t ell_len);
int format_string(const char *fmt, va_list args, char *buf, size_t buf_size);
int format_and_print(const char *fmt, va_list args, void (*printer)(const char*));
bool term_check_size(int x, int y); // проверить
void fcoor_curs_x(int x);
void txt_ansi(const char* fmt);
void apply_attr_curs(int attr);
void apply_coord_curs(int x, int y);
void apply_fg_ansi_016(int fg);
void apply_fg_ansi_256(int fg);
void apply_bg_ansi_256(int bg);
void apply_bg_ansi_016(int bg);

#endif
