//#include <ncurses.h>

#include "../libtermcolor/libtermcolor.h"
#include "libline.h"

// ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI

void generic_msg_ansi(int mod, int x, int y, int attr, int fg, int bg, const char* fmt, ...){
    x_y_ansi(x, y);
    attr_ansi(attr);
    mod_fg_ansi(mod, fg);
    mod_bg_ansi(mod, bg);
    FORMAT_AND_PRINT(fmt, txt_ansi);
    reset_ansi(1);
}

// NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES

void generic_msg_ncurses(int x, int y, int attr, int fg, int bg, const char* fmt, ...){
              apply_coord_curs(x, y);
              apply_attr_curs(attr);
              attron(COLOR_PAIR(get_color_pair(fg, bg)));
              FORMAT_AND_PRINT(fmt, txt_curs);
              attroff(COLOR_PAIR(get_color_pair(fg, bg)));
              refresh();
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
    attrset(A_NORMAL);
    refresh();
}

//==============================================
void win_msg_ncurses(
    WINDOW *win,
    int x, int y,
    int width, int height,
    int attr,
    int fg, int bg,
    int bgsingl,
    int title_align, int title_int,
    FrameStyleType style_type,
    const char *fmt, ...)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (x < 0) x = 0;
    if (y < 1) y = 1;
    attr_t attr_flag = convert_attr_to_ncurses(attr);
    int pair_bg      = get_color_pair(fg, bgsingl);
    int pair_bgsingl = get_color_pair(fg, bg);
    wattron(win, COLOR_PAIR(pair_bgsingl));
    wmove(win, y - 1, x - 1);
    for (int i = 0; i < width; i++) {
        waddch(win, ' ');
    }
    wmove(win, y - 1, x - 1);
    wattron(win, COLOR_PAIR(pair_bgsingl));
    if (fmt && fmt[0]) {
        va_list args;
        va_start(args, fmt);
        wchar_t *wtitle = NULL;
        int title_width = 0;
        int title_start = 0;
        if (prepare_frame_title_common(
                fmt,
                args,
                width,
                x,
                title_align,
                title_int,
                &wtitle,
                &title_width,
                &title_start) == 0)
        {
            wmove(win, y - 1, title_start - 1);
            char display_title[512];
            wcstombs(display_title, wtitle, sizeof(display_title));
            wattron(win, attr_flag);
            wattron(win, COLOR_PAIR(pair_bg));
            wprintw(win, "%s", display_title);
            wattroff(win, COLOR_PAIR(pair_bg));
            wattroff(win, attr_flag);
            free(wtitle);
        }
        va_end(args);
    }
    wattron(win, COLOR_PAIR(pair_bgsingl));
    wmove(win, y - 1, x + width - 2);
    wattroff(win, COLOR_PAIR(pair_bgsingl));
    wnoutrefresh(win);
    (void)height;      // параметр принят, но для одной строки не нужен
    (void)w;           // параметр принят для совместимости
    (void)style_type;   // <--- добавить эту строку
}
