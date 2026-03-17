#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <ncurses.h>
#include <locale.h>
#include <strings.h>
#include <wchar.h>
#include <stdarg.h>
#include <ctype.h>

#include "pseudographic_font_3.h"
#include "pseudographic_font_5.h"
#include "pseudographic_font_7.h"
#include "pseudographic_font_9.h"
#include "libtermcolor.h"
#include "tc_frame.h"
#include "tc_file_types.h"
#include "tc_file_directories_color.h"

#define FORMAT_AND_PRINT(fmt, printer) do {va_list args; va_start(args, fmt); format_and_print(fmt, args, printer); va_end(args);} while(0)

static bool locale_set = false;
static bool colors_started = false;
static ColorPairEntry pairs[MAX_PAIRS];
static int num_pairs = 0;
static short next_pair = 1;
int term_height = 0;

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

static void txt_curs(const char* fmt) {
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
// CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS CROSS-MODE FUNCTIONS
int get_char_width(wchar_t c) {
    int w = wcwidth(c);
    return (w >= 0) ? w : 1;
}

int compute_wchar_width(const wchar_t *w, size_t len) {
    int width = 0;
    for (size_t i = 0; i < len; i++) {
        int char_width = get_char_width(w[i]);
        if (char_width >= 0) width += char_width;
    }
    return width;
}

int calculate_visual_width(const char *src) {
    if (!src) return 0;
    wchar_t *wsrc = NULL;
    size_t wlen = 0;
    if (convert_to_wchar(src, &wsrc, &wlen) != 0) return 0;
    int total_width = compute_wchar_width(wsrc, wlen);
    free(wsrc);
    return total_width;
}

int convert_to_wchar(const char *src, wchar_t **wsrc_out, size_t *wlen_out) {
    if (!src || !wsrc_out || !wlen_out) {
        if (wsrc_out) *wsrc_out = NULL;
        if (wlen_out) *wlen_out = 0;
        return -1;
    }
    size_t src_len = strlen(src);
    wchar_t *wsrc = calloc(src_len + 1, sizeof(wchar_t));
    if (!wsrc) {
        return -1;
    }
    mbstate_t state = {0};
    const char *ptr = src;
    size_t wlen = mbsrtowcs(wsrc, &ptr, src_len + 1, &state);
    if (wlen == (size_t)-1) {
        free(wsrc);
        wsrc = calloc(src_len + 1, sizeof(wchar_t));
        if (!wsrc) return -1;
        for (size_t i = 0; i < src_len; i++) {
            wsrc[i] = (wchar_t)src[i];
        }
        wsrc[src_len] = L'\0';
        wlen = src_len;
    }
    *wsrc_out = wsrc;
    *wlen_out = wlen;
    return 0;
}

static void add_ellipsis(wchar_t *dest, size_t *dest_idx, size_t dest_size, const wchar_t *ellipsis, size_t ell_len) {
    for (size_t j = 0; j < ell_len && *dest_idx < dest_size - 1; j++) {
        dest[(*dest_idx)++] = ellipsis[j];
    }
}

int prepare_display_wstring(const char *src, int max_visual_width, wchar_t *dest, size_t dest_size, int add_suffix, const wchar_t *ellipsis, int visual_offset, int use_middle_ellipsis) {
    if (!src || !dest || dest_size == 0) return -1;
    dest[0] = L'\0';
    if (strlen(src) == 0) return 0;
    if (!ellipsis) ellipsis = L"..";
    wchar_t *wsrc = NULL;
    size_t wlen = 0;
    if (convert_to_wchar(src, &wsrc, &wlen) != 0) return -1;
    int total_width = calculate_visual_width(src);
    if (total_width <= max_visual_width) {
        wcsncpy(dest, wsrc, dest_size - 1);
        dest[dest_size - 1] = L'\0';
        free(wsrc);
        return 0;
    }
    size_t ell_len = wcslen(ellipsis);
    int ell_width = compute_wchar_width(ellipsis, ell_len);
    int remaining_width = max_visual_width - ell_width;
    if (remaining_width < 2) {
        wcsncpy(dest, ellipsis, dest_size - 1);
        dest[dest_size - 1] = L'\0';
        free(wsrc);
        return 0;
    }
    size_t dest_idx = 0;
    int current_width = 0;
    if (use_middle_ellipsis) {
        int front_width = remaining_width / 2;
        int back_width = remaining_width - front_width;
        size_t i = 0;
        while (i < wlen && dest_idx < dest_size - 1 && current_width < front_width) {
            int char_width = get_char_width(wsrc[i]);
            if (current_width + char_width > front_width) break;
            dest[dest_idx++] = wsrc[i++];
            current_width += char_width;
        }
        add_ellipsis(dest, &dest_idx, dest_size, ellipsis, ell_len);
        current_width += ell_width;
        size_t back_start = wlen;
        int back_current = 0;
        while (back_start > i && back_current < back_width) {
            back_start--;
            int char_width = get_char_width(wsrc[back_start]);
            if (back_current + char_width > back_width) {
                back_start++;
                break;
            }
            back_current += char_width;
        }
        for (size_t k = back_start; k < wlen && dest_idx < dest_size - 1; k++) {
            dest[dest_idx++] = wsrc[k];
        }
    } else {
        if (visual_offset < 0) visual_offset = 0;
        if (visual_offset > total_width) visual_offset = total_width;
        int current_offset = 0;
        size_t start_i = 0;
        while (start_i < wlen) {
            int char_width = get_char_width(wsrc[start_i]);
            if (current_offset + char_width > visual_offset) break;
            current_offset += char_width;
            start_i++;
        }
        int need_end_ellipsis = (start_i < wlen && total_width - current_offset > max_visual_width);
        if (need_end_ellipsis) {
            remaining_width = max_visual_width - ell_width;
            if (remaining_width < 0) remaining_width = 0;
        } else {
            remaining_width = max_visual_width;
        }
        size_t i = start_i;
        while (i < wlen && dest_idx < dest_size - 1) {
            int char_width = get_char_width(wsrc[i]);
            if (current_width + char_width > remaining_width) break;
            dest[dest_idx++] = wsrc[i++];
            current_width += char_width;
        }
        if (need_end_ellipsis) {
            add_ellipsis(dest, &dest_idx, dest_size, ellipsis, ell_len);
        }
    }
    dest[dest_idx] = L'\0';
    if (add_suffix) {
        wchar_t suffix = L'/';
        int suffix_width = get_char_width(suffix);
        if (current_width + suffix_width <= max_visual_width && dest_idx < dest_size - 1) {
            dest[dest_idx++] = suffix;
            dest[dest_idx] = L'\0';
        }
    }
    free(wsrc);
    return 0;
}

static int format_string(const char *fmt, va_list args, char *buf, size_t buf_size) {
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

static int format_and_print(const char *fmt, va_list args, void (*printer)(const char*)) {
    char buf[256];
    if (format_string(fmt, args, buf, sizeof(buf)) != 0) {
        return -1;
    }
    printer(buf);
    return 0;
}
// NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES NCURSES
static bool term_check_size(int x, int y) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        return false;
    }
    return (x > 0 && x <= ws.ws_col && y > 0 && y <= ws.ws_row);
}

static void fcoor_curs_x(int x) {
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

static void apply_coord_curs(int x, int y) {
    if (x != VOID && y != VOID) {
        fcoor_curs_xy(x, y);
    } else {
        if (x != VOID) fcoor_curs_x(x);
        if (y != VOID) fcoor_curs_y(y);
    }
}

static void apply_attr_curs(int attr) {
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

// ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI ANSI
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

static void txt_ansi(const char* fmt) {
    if (fmt != NULL && fmt[0] != '\0') {
        printf("%s", fmt);
    }
}
// fg
static void apply_fg_ansi_016(int fg) {
    if (fg == VOID) return;
    printf("\033[39m");
    if ((fg >= 30 && fg <= 37) || (fg >= 90 && fg <= 97) || (fg == 39)) {
        printf("\033[%dm", fg);
    }
}

static void apply_fg_ansi_256(int fg) {
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
// bg
static void apply_bg_ansi_256(int bg) {
    if (bg == VOID) return;
        printf("\033[49m");
    if (bg >= 0 && bg <= 255) {
        printf("\033[48;5;%dm", bg);
    }
}

static void apply_bg_ansi_016(int bg) {
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

void var_generic_msg_ncurses(int y, int attr, int fg, int bg, ...);
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
