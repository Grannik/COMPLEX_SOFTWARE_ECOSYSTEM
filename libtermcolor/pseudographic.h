#ifndef PSEUDOGRAPHIC_H
#define PSEUDOGRAPHIC_H
#include "libtermcolor.h"

void generic_abc_5_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_7_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_5_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_7_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void group_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void generic_abc_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void group_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void generic_abc_9_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void generic_abc_9_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void var_abc_9_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void generic_cycle_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...);

#endif
