#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <ncurses.h>
#include <locale.h>
#include <strings.h>
#include <wchar.h>
#include <stdarg.h>
#include <ctype.h>

#include "libtermcolor.h"
#include "pseudographic_font_3.h"
#include "pseudographic_font_5.h"
#include "pseudographic_font_7.h"
#include "pseudographic_font_9.h"
#include "pseudographic.h"
#include "tc_file_types.h"

#define FORMAT_AND_PRINT(fmt, printer) do {va_list args; va_start(args, fmt); format_and_print(fmt, args, printer); va_end(args);} while(0)

static const char** get_pseudographic_char_generic(
    const char** font,
    int height,
    const char** empty,
    char c)
{
    int idx = -1;

    if      (c >= '0' && c <= '9')  idx = c - '0';
    else if (c >= 'A' && c <= 'Z')  idx = 10 + (c - 'A');
    else if (c >= 'a' && c <= 'z')  idx = 10 + (c - 'a');
    else if (c == '.')              idx = 36;
    else if (c == ':')              idx = 37;
    else if (c == '/')              idx = 38;
    else if (c == '\\')             idx = 39;
    else if (c == '(')              idx = 40;
    else if (c == ')')              idx = 41;
    if (idx < 0 || idx >= 42) {
        return empty;
    }

    return font + (idx * height);
}

void generic_msg_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...){
              apply_coord_curs(x, y);
              apply_attr_curs(attr);
              attron(COLOR_PAIR(get_color_pair(fg, bg)));
              FORMAT_AND_PRINT(fmt, txt_curs);
              attroff(COLOR_PAIR(get_color_pair(fg, bg)));
              refresh();
}

void generic_abc_5_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
    const char** chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_5[0][0], 5, empty_char_5, buf[i]);
    }
    attr_t attr_flag = convert_attr_to_ncurses(attr);
    int color_pair = get_color_pair(fg, bg);
    for (int row = 0; row < 5; row++) {
        wmove(win, y + row - 1, x - 1);
        wattron(win, attr_flag | COLOR_PAIR(color_pair));
        for (size_t i = 0; i < len; i++) {
            wprintw(win, "%s", chars[i][row]);
        }
        wattroff(win, attr_flag | COLOR_PAIR(color_pair));
    }
    wnoutrefresh(win);
    doupdate();
}

void generic_abc_7_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
       const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_7[0][0], 8, empty_char_7, buf[i]);
    }
    attr_t attr_flag = convert_attr_to_ncurses(attr);
    int color_pair = get_color_pair(fg, bg);
    for (int row = 0; row < 8; row++) {
        wmove(win, y + row - 1, x - 1);
        wattron(win, attr_flag | COLOR_PAIR(color_pair));
        for (size_t i = 0; i < len; i++) {
            wprintw(win,   "%s", chars[i][row]);
        }
        wattroff(win, attr_flag | COLOR_PAIR(color_pair));
    }
    wnoutrefresh(win);
    doupdate();
}

void generic_msg_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...){
    x_y_ansi(x, y);
    attr_ansi(attr);
    mod_fg_ansi(mod, fg);
    mod_bg_ansi(mod, bg);
    FORMAT_AND_PRINT(fmt, txt_ansi);
    reset_ansi(1);
}

void generic_abc_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
    const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_3[0][0], 3, empty_char, buf[i]);
    }
    for (int row = 0; row < 3; row++) {
        printf("\033[%d;%dH", y + row, x);
        if (attr != VOID) attr_ansi(attr);
        if (mod == 16) {
            if (fg != VOID) apply_fg_ansi_016(fg);
            if (bg != VOID) apply_bg_ansi_016(bg);
        } else if (mod == 256) {
            if (fg != VOID) apply_fg_ansi_256(fg);
            if (bg != VOID) apply_bg_ansi_256(bg);
        }
        for (size_t i = 0; i < len; i++) {
            printf("%s", chars[i][row]);
        }
    }
    reset_ansi(1);
}

void generic_abc_5_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
    const char** chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_5[0][0], 5, empty_char_5, buf[i]);
    }
    for (int row = 0; row < 5; row++) {
        printf("\033[%d;%dH", y + row, x);
        if (attr != VOID) attr_ansi(attr);
        if (mod == 16) {
            if (fg != VOID) apply_fg_ansi_016(fg);
            if (bg != VOID) apply_bg_ansi_016(bg);
        } else if (mod == 256) {
            if (fg != VOID) apply_fg_ansi_256(fg);
            if (bg != VOID) apply_bg_ansi_256(bg);
        }
        for (size_t i = 0; i < len; i++) {
            printf("%s", chars[i][row]);
        }
    }
    reset_ansi(1);
}

void generic_abc_7_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
       const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_7[0][0], 8, empty_char_7, buf[i]);
    }
    for (int row = 0; row < 8; row++) {
        printf("\033[%d;%dH", y + row, x);
        if (attr != VOID) attr_ansi(attr);
        if (mod == 16) {
            if (fg != VOID) apply_fg_ansi_016(fg);
            if (bg != VOID) apply_bg_ansi_016(bg);
        }
        else if (mod == 256) {
            if (fg != VOID) apply_fg_ansi_256(fg);
            if (bg != VOID) apply_bg_ansi_256(bg);
        }
        for (size_t i = 0; i < len; i++) {
            printf("%s", chars[i][row]);
        }
    }
    reset_ansi(1);
}

void group_generic_msg_ncurses(int y, int attr, int fg, int bg, Message *messages, int count)
{
    if (!messages || count <= 0) return;

    apply_attr_curs(attr);
    attron(COLOR_PAIR(get_color_pair(fg, bg)));

    for (int i = 0; i < count; i++) {
        apply_coord_curs(messages[i].x, y);
        txt_curs(messages[i].text);
    }
    attroff(COLOR_PAIR(get_color_pair(fg, bg)));
    refresh();
}

void group_msg(int y, int attr, int fg, int bg, ...)
{
    Message msgs[16];
    int count = 0;
    va_list args;
    va_start(args, bg);
    while (count < 15) {
        int x = va_arg(args, int);
        const char* t = va_arg(args, const char*);
        if (x == -1 && t == (const char*)-1) break;

        msgs[count].x = x;
        msgs[count].text = t;
        count++;
    }
    va_end(args);
    if (count > 0)
        group_generic_msg_ncurses(y, attr, fg, bg, msgs, count);
}

void var_generic_msg_ncurses(int y, int attr, int fg, int bg, ...) {
    if (y == VOID) return;
    apply_attr_curs(attr);
    attron(COLOR_PAIR(get_color_pair(fg, bg)));
    va_list args;
    va_start(args, bg);
    char buf[256];
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* fmt = va_arg(args, const char*);
        if (!fmt) continue;
        enum { TYPE_UNKNOWN, TYPE_INT, TYPE_STRING } arg_type = TYPE_UNKNOWN;
        const char* p = fmt;
        while (*p) {
            if (*p == '%') {
                p++;
                while (*p && (isdigit(*p) || *p == '0' || *p == '.' || *p == '-' || *p == '+' || *p == ' ' || *p == '#')) p++;
                char spec = *p;
                if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    arg_type = TYPE_INT;
                } else if (spec == 's') {
                    arg_type = TYPE_STRING;
                }
                break;
            }
            p++;
        }

        int written = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        switch (arg_type) {
            case TYPE_INT: {
                int val = va_arg(args, int);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            case TYPE_STRING: {
                const char* val = va_arg(args, const char*);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            default:
                continue;
        }
#pragma GCC diagnostic pop

        if (written < 0 || (size_t)written >= sizeof(buf)) {
        }

        apply_coord_curs(x, y);
        txt_curs(buf);
    }

    va_end(args);
    attroff(COLOR_PAIR(get_color_pair(fg, bg)));
    refresh();
}

void group_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* str = va_arg(args, const char*);
        if (str == (const char*)-1) break;
        generic_abc_ncurses(win, x, y, attr, fg, bg, "%s", str);
    }
    va_end(args);
}

void var_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    char buf[256];
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* fmt = va_arg(args, const char*);
        if (!fmt) continue;
        enum { TYPE_UNKNOWN, TYPE_INT, TYPE_STRING } arg_type = TYPE_UNKNOWN;
        const char* p = fmt;
        while (*p) {
            if (*p == '%') {
                p++;
                while (*p && (isdigit(*p) || *p == '0' || *p == '.' || *p == '-' || *p == '+' || *p == ' ' || *p == '#')) p++;
                char spec = *p;
                if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    arg_type = TYPE_INT;
                } else if (spec == 's') {
                    arg_type = TYPE_STRING;
                }
                break;
            }
            p++;
        }
        int written = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        switch (arg_type) {
            case TYPE_INT: {
                int val = va_arg(args, int);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            case TYPE_STRING: {
                const char* val = va_arg(args, const char*);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            default:
                strncpy(buf, fmt, sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                break;
        }
#pragma GCC diagnostic pop
        if (arg_type != TYPE_UNKNOWN && (written < 0 || (size_t)written >= sizeof(buf))) {
            continue;
        }
        generic_abc_ncurses(win, x, y, attr, fg, bg, "%s", buf);
    }
    va_end(args);
}

void generic_abc_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;

    const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_3[0][0], 3, empty_char, buf[i]);
    }

    attr_t attr_flag = convert_attr_to_ncurses(attr);
    int color_pair = get_color_pair(fg, bg);

    for (int row = 0; row < 3; row++) {
        wmove(win, y + row - 1, x - 1);
        wattron(win, attr_flag | COLOR_PAIR(color_pair));
        for (size_t i = 0; i < len; i++) {
            wprintw(win, "%s", chars[i][row]);
        }
        wattroff(win, attr_flag | COLOR_PAIR(color_pair));
    }
    wnoutrefresh(win);
    doupdate();
}

void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* str = va_arg(args, const char*);
        if (str == (const char*)-1) break;
        generic_abc_5_ncurses(win, x, y, attr, fg, bg, "%s", str);
    }
    va_end(args);
}

void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    char buf[256];
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* fmt = va_arg(args, const char*);
        if (!fmt) continue;

        enum { TYPE_UNKNOWN, TYPE_INT, TYPE_STRING } arg_type = TYPE_UNKNOWN;
        const char* p = fmt;
        while (*p) {
            if (*p == '%') {
                p++;
                while (*p && (isdigit(*p) || *p == '0' || *p == '.' || *p == '-' || *p == '+' || *p == ' ' || *p == '#')) p++;
                char spec = *p;
                if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    arg_type = TYPE_INT;
                } else if (spec == 's') {
                    arg_type = TYPE_STRING;
                }
                break;
            }
            p++;
        }

        int written = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        switch (arg_type) {
            case TYPE_INT: {
                int val = va_arg(args, int);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            case TYPE_STRING: {
                const char* val = va_arg(args, const char*);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            default:
                strncpy(buf, fmt, sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                break;
        }
#pragma GCC diagnostic pop

        if (arg_type != TYPE_UNKNOWN && (written < 0 || (size_t)written >= sizeof(buf))) {
            continue;
        }
        generic_abc_5_ncurses(win, x, y, attr, fg, bg, "%s", buf);
    }
    va_end(args);
}

void group_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* str = va_arg(args, const char*);
        if (str == (const char*)-1) break;
        generic_abc_7_ncurses(win, x, y, attr, fg, bg, "%s", str);
    }
    va_end(args);
}

void var_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    char buf[256];
    while (true) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* fmt = va_arg(args, const char*);
        if (!fmt) continue;

        enum { TYPE_UNKNOWN, TYPE_INT, TYPE_STRING } arg_type = TYPE_UNKNOWN;
        const char* p = fmt;
        while (*p) {
            if (*p == '%') {
                p++;
                while (*p && (isdigit(*p) || *p == '0' || *p == '.' || *p == '-' || *p == '+' || *p == ' ' || *p == '#')) p++;
                char spec = *p;
                if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    arg_type = TYPE_INT;
                } else if (spec == 's') {
                    arg_type = TYPE_STRING;
                }
                break;
            }
            p++;
        }

        int written = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        switch (arg_type) {
            case TYPE_INT: {
                int val = va_arg(args, int);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            case TYPE_STRING: {
                const char* val = va_arg(args, const char*);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            default:
                strncpy(buf, fmt, sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                break;
        }
#pragma GCC diagnostic pop

        if (arg_type != TYPE_UNKNOWN && (written < 0 || (size_t)written >= sizeof(buf))) {
            continue;
        }
        generic_abc_7_ncurses(win, x, y, attr, fg, bg, "%s", buf);
    }
    va_end(args);
}

void generic_abc_9_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;
       const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_9[0][0], 11, empty_char_9, buf[i]);
        }
    for (int row = 0; row < 9; row++) {
        printf("\033[%d;%dH", y + row, x);
        if (attr != VOID)
            attr_ansi(attr);
        if (mod == 16) {
            if (fg != VOID) apply_fg_ansi_016(fg);
            if (bg != VOID) apply_bg_ansi_016(bg);
        }
        else if (mod == 256) {
            if (fg != VOID) apply_fg_ansi_256(fg);
            if (bg != VOID) apply_bg_ansi_256(bg);
        }
        for (size_t i = 0; i < len; i++) {
            printf("%s", chars[i][row]);
        }
    }
    reset_ansi(1);
}

void generic_abc_9_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    size_t len = strlen(buf);
    if (len == 0) return;

    const char **chars[256];
    for (size_t i = 0; i < len; i++) {
        chars[i] = get_pseudographic_char_generic((const char**)&font_9[0][0], 11, empty_char_9, buf[i]);
    }

    attr_t attr_flag = convert_attr_to_ncurses(attr);
    int color_pair = get_color_pair(fg, bg);

    for (int row = 0; row < 9; row++) {
        wmove(win, y + row - 1, x - 1);
        wattron(win, attr_flag | COLOR_PAIR(color_pair));
        for (size_t i = 0; i < len; i++) {
            wprintw(win, "%s", chars[i][row]);
        }
        wattroff(win, attr_flag | COLOR_PAIR(color_pair));
    }
    wnoutrefresh(win);
    doupdate();
}

void var_abc_9_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...) {
    va_list args;
    va_start(args, bg);
    char buf[256];
    while (1) {
        int x = va_arg(args, int);
        if (x == -1) break;
        const char* fmt = va_arg(args, const char*);
        if (!fmt) continue;

        enum { TYPE_UNKNOWN, TYPE_INT, TYPE_STRING } arg_type = TYPE_UNKNOWN;
        const char* p = fmt;
        while (*p) {
            if (*p == '%') {
                p++;
                while (*p && (isdigit(*p) || *p == '0' || *p == '.' || *p == '-' || *p == '+' || *p == ' ' || *p == '#')) p++;
                char spec = *p;
                if (spec == 'd' || spec == 'i' || spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X') {
                    arg_type = TYPE_INT;
                } else if (spec == 's') {
                    arg_type = TYPE_STRING;
                }
                break;
            }
            p++;
        }

        int written = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        switch (arg_type) {
            case TYPE_INT: {
                int val = va_arg(args, int);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            case TYPE_STRING: {
                const char* val = va_arg(args, const char*);
                written = snprintf(buf, sizeof(buf), fmt, val);
                break;
            }
            default:
                strncpy(buf, fmt, sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                break;
        }
#pragma GCC diagnostic pop

        if (arg_type != TYPE_UNKNOWN && (written < 0 || (size_t)written >= sizeof(buf))) {
            continue;
        }

        generic_abc_9_ncurses(win, x, y, attr, fg, bg, "%s", buf);
    }
    va_end(args);
}

void generic_cycle_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...){
    if (x != VOID && y != VOID) {
        move(y, x);
    } else {
        if (x != VOID) {
            int cur_y = getcury(stdscr);
            move(cur_y, x);
        }
        if (y != VOID) {
            int cur_x = getcurx(stdscr);
            move(y, cur_x);
        }
    }
    apply_attr_curs(attr);
    attron(COLOR_PAIR(get_color_pair(fg, bg)));
    FORMAT_AND_PRINT(fmt, txt_curs);
    attroff(COLOR_PAIR(get_color_pair(fg, bg)));
}
