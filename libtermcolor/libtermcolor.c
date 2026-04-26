#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <ncurses.h>
#include <locale.h>
#include <strings.h>
#include <wchar.h>
#include <stdarg.h>
#include <ctype.h>

#include "pseudographic.h"
#include "../libstrprepare/libstrprepare.h"
#include "libtermcolor.h"
#include "tc_frame.h"
#include "tc_file_types.h"
#include "tc_file_directories_color.h"

static bool locale_set = false;
static bool colors_started = false;
static ColorPairEntry pairs[MAX_PAIRS];
static int num_pairs = 0;
static short next_pair = 1;
int term_height = 0;

 attr_t convert_attr_to_ncurses(int attr) {
    if (attr == VOID) return 0;
    attr_t result = 0;
    if (attr & 1)  result |= A_BOLD;
    if (attr & 2)  result |= A_UNDERLINE;
    if (attr & 4)  result |= A_BLINK;
    if (attr & 8)  result |= A_DIM;
    if (attr & 16) result |= A_STANDOUT;
    #ifdef A_ITALIC
    if (attr & 32) result |= A_ITALIC;
    #endif
    return result;
}

void color_init(void) {
    if (!locale_set) {
        setlocale(LC_ALL, "");
        locale_set = true;
    }
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
        term_height = ws.ws_row;
}

void color_info(void) {
    if (has_colors() && COLORS > 0) {
        mvprintw(4, 0, "COLORS = %d, COLOR_PAIRS = %d", COLORS, COLOR_PAIRS);
    } else {
        mvprintw(4, 0, "COLORS = %d, COLOR_PAIRS = %d (colors not available)", COLORS, COLOR_PAIRS);
    }
}

void txt_curs(const char* fmt) {
    if (fmt != NULL && fmt[0] != '\0') {
        printw("%s", fmt);
    }
}

typedef struct WinDimensions {
    int width;
    int height;
    int start_y;
    int start_x;
} WinDimensions;

 int format_string(const char *fmt, va_list args, char *buf, size_t buf_size) {
    if (!fmt || !buf || buf_size == 0) {
        if (buf) buf[0] = '\0';
        return -1;
    }
    int result = vsnprintf(buf, buf_size, fmt, args);
    if (result < 0) {
        buf[buf_size - 1] = '\0';
        return -1;
    }
    if ((size_t)result >= buf_size) {
        buf[buf_size - 1] = '\0';
        return -1;
    }
    return 0;
}

 int format_and_print(const char *fmt, va_list args, void (*printer)(const char*)) {
    char buf[512];
    if (format_string(fmt, args, buf, sizeof(buf)) != 0) {
        return -1;
    }
    printer(buf);
    return 0;
}
// NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES
 bool term_check_size(int x, int y) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        return false;
    }
    return (x > 0 && x <= ws.ws_col && y > 0 && y <= ws.ws_row);
}

 void fcoor_curs_x(int x) {
    if (x == VOID) return;
    int cur_y = getcury(stdscr);
    move(cur_y, x - 1);
}

static void fcoor_curs_y(int y) {
    if (y == VOID) return;
    int cur_x = getcurx(stdscr);
    move(y - 1, cur_x);
}

static void fcoor_curs_xy(int x, int y) {
    if (x == VOID || y == VOID) return;
       move(y - 1, x - 1);
}

 void apply_coord_curs(int x, int y) {
    if (x != VOID && y != VOID) {
        fcoor_curs_xy(x, y);
    } else {
        if (x != VOID) fcoor_curs_x(x);
        if (y != VOID) fcoor_curs_y(y);
    }
}

 void apply_attr_curs(int attr) {
    if (attr == -1) {
        attrset(A_NORMAL);
        return;
    }
    attr_t attr_flag = convert_attr_to_ncurses(attr);
    attrset((int)attr_flag);
}

int get_color_pair(int fg, int bg) {
    if (!colors_started) {
        if (has_colors()) {
            start_color();
            use_default_colors();
        } else {
            return 0;
        }
        colors_started = true;
    }
    if ((fg < -1 || fg >= COLORS) ||
        (bg < -1 || bg >= COLORS)) {
        return 0;
    }
    for (int i = 0; i < num_pairs; i++) {
        if (pairs[i].fg == fg && pairs[i].bg == bg) {
            return pairs[i].pair;
        }
    }
    if (num_pairs >= MAX_PAIRS || next_pair >= COLOR_PAIRS) {
        return 0;
    }
    short p = next_pair++;
    init_pair(p, (short)fg, (short)bg);
    pairs[num_pairs].fg = (short)fg;
    pairs[num_pairs].bg = (short)bg;
    pairs[num_pairs].pair = p;
    num_pairs++;
    return p;
}

void x_y_ansi(int x, int y) {
    if (x == VOID || y == VOID) return;
    if (term_check_size(x, y)) {
        printf("\033[%d;%dH", y, x);
    }
}

 void attr_ansi(int attr) {
    if (attr == VOID) return;
    for (int i = 22; i <= 29; i++) {
        if (i != 26) {
            printf("\033[%dm", i);
        }
    }
        if ((attr >= 1 && attr <= 9) || (attr >= 22 && attr <= 29)) {
           printf("\033[%dm", attr);
    }
}

 void txt_ansi(const char* fmt) {
    if (fmt != NULL && fmt[0] != '\0') {
        printf("%s", fmt);
    }
}

 void apply_fg_ansi_016(int fg) {
    if (fg == VOID) return;
    printf("\033[39m");
    if ((fg >= 30 && fg <= 37) || (fg >= 90 && fg <= 97) || (fg == 39)) {
        printf("\033[%dm", fg);
    }
}

 void apply_fg_ansi_256(int fg) {
    if (fg == VOID) return;
    printf("\033[39m");
    if (fg >= 0 && fg <= 255) {
        printf("\033[38;5;%dm", fg);
    }
}

 void mod_fg_ansi(int mod, int fg) {
           if (mod == 16) {
              apply_fg_ansi_016(fg);
    } else if (mod == 256) {
              apply_fg_ansi_256(fg);
    } else if (mod == VOID) {
}
}

 void apply_bg_ansi_256(int bg) {
    if (bg == VOID) return;
        printf("\033[49m");
    if (bg >= 0 && bg <= 255) {
        printf("\033[48;5;%dm", bg);
    }
}

 void apply_bg_ansi_016(int bg) {
    if (bg == VOID) return;
        printf("\033[49m");
    if ((bg >= 40 && bg <= 47) || (bg >= 100 && bg <= 107) || (bg == 49)) {
        printf("\033[%dm", bg);
    }
}

 void mod_bg_ansi(int mod, int bg) {
           if (mod == 16) {
              apply_bg_ansi_016(bg);
    } else if (mod == 256) {
              apply_bg_ansi_256(bg);
    } else if (mod == VOID) {
}
}

void reset_ansi(int flag) {
    if (flag == 1) {
        printf("\033[0m\n");
        fflush(stdout);
    }
    else if (flag == 2) {
        printf("\033[2J\033[H");
        fflush(stdout);
    }
    else if (flag == 3) {
        printf("\033[r");
        fflush(stdout);
    }
}
