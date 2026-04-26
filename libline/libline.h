#ifndef LIBLINE_H
#define LIBLINE_H

#include <stdarg.h>
#include <ctype.h>
//#include <ncurses.h>
#include <ncursesw/ncurses.h>
#include "../libtermcolor/libtermcolor.h"   // <--- добавить сюда

#define FORMAT_AND_PRINT(fmt, printer) do {va_list args; va_start(args, fmt); format_and_print(fmt, args, printer); va_end(args);} while(0)

void generic_msg_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...);

typedef struct Message {
    int x;
    const char *text;
} Message;

void generic_msg_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...);
void group_generic_msg_ncurses(int y, int attr, int fg, int bg, Message *messages, int count);
void group_msg(int y, int attr, int fg, int bg, ...);
void var_generic_msg_ncurses(int y, int attr, int fg, int bg, ...);

void win_msg_ncurses(
    WINDOW *win,
    int x, int y,
    int width, int height,
    int attr,
    int fg, int bg,
    int bgsingl,
    int title_align, int title_int,
    FrameStyleType style_type,
    const char *fmt, ...);

#endif
