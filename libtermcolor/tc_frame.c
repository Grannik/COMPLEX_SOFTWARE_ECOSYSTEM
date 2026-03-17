#define NCURSES_WIDECHAR 1
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include "libtermcolor.h"
#include "tc_frame.h"

struct FrameStyle {
    const char *top_left;
    const char *top_right;
    const char *bottom_left;
    const char *bottom_right;
    const char *horizontal;
    const char *vertical;
    const char *title_left;
    const char *title_right;
};

 const FrameStyle FRAME_STYLES[FRAME_STYLE_COUNT] = {
    [FRAME_SINGLE] = { "┌","┐","└","┘","─","│","┤","├" },
    [FRAME_DOUBLE] = { "╔","╗","╚","╝","═","║","╡","╞" },
    [FRAME_HEAVY]  = { "┏","┓","┗","┛","━","┃","┫","┣" },
    [FRAME_ROUNDED]= { "╭","╮","╰","╯","─","│","┤","├" }
};

 const FrameStyle* get_frame_style(FrameStyleType type)
{
    if (type < 0 || type >= FRAME_STYLE_COUNT)
        return &FRAME_STYLES[FRAME_SINGLE];
    return &FRAME_STYLES[type];
}

 void get_module_pos(char *symbol, int *pos) {
    if (*symbol >= '0' && *symbol <= '9') {
        *pos = *symbol - '0';
    } else if (*symbol >= 'a' && *symbol <= 'z') {
        *pos = 10 + (*symbol - 'a');
    } else if (*symbol >= 'A' && *symbol <= 'Z') {
        *pos = 10 + (*symbol - 'A');
    } else {
        *pos = 0;
        *symbol = '0';
    }
}

 int calculate_title_start(int x, int width, int title_block_len, int title_align, int title_int)
{
    int title_start;
    if (title_align == 0) {
        title_start = x + 2 + title_int;
    } else if (title_align == 1) {
        title_start = x + (width - title_block_len) / 2;
    } else {
        title_start = x + width - title_block_len - 1 + title_int;
    }
    int min_start = x + 1;
    int max_end = x + width - 2;
    int title_end = title_start + title_block_len - 1;
    if (title_start < min_start) {
        title_start = min_start;
    }
    if (title_end > max_end) {
        title_start = max_end - title_block_len + 1;
    }
    return title_start;
}

int prepare_frame_title(
    const char *fmt, va_list args, int max_visual_width,
    const wchar_t *ellipsis, int visual_offset, int use_middle_ellipsis,
    wchar_t **wtitle_out,
    int *title_width_out,
    int *title_block_len_out)
{
    if (wtitle_out)        *wtitle_out        = NULL;
    if (title_width_out)   *title_width_out   = 0;
    if (title_block_len_out) *title_block_len_out = 0;
    if (!fmt || fmt[0] == '\0') {
        return 0;
    }
    if (max_visual_width < 1) {
        max_visual_width = 1;
    }
    char title_buf[512] = {0};
    if (vsnprintf(title_buf, sizeof(title_buf), fmt, args) < 0) {
        return -1;
    }
    wchar_t wtitle[512] = {0};
    int prep_res = prepare_display_wstring(
        title_buf,
        max_visual_width,
        wtitle,
        sizeof(wtitle) / sizeof(wchar_t),
        1,
        ellipsis ? ellipsis : L"…",
        visual_offset,
        use_middle_ellipsis ? 1 : 0
    );
    if (prep_res != 0) {
        return -1;
    }
    size_t wlen = wcslen(wtitle);
    int title_width = compute_wchar_width(wtitle, wlen);
    int title_block_len = title_width + 4;
    if (wtitle_out) {
        size_t len = wcslen(wtitle) + 1;
        *wtitle_out = malloc(len * sizeof(wchar_t));
        if (!*wtitle_out) {
            return -1;
        }
        wcscpy(*wtitle_out, wtitle);
    }
    if (title_width_out) {
        *title_width_out = title_width;
    }
    if (title_block_len_out) {
        *title_block_len_out = title_block_len;
    }
    return 0;
}

 int prepare_frame_title_common(const char *fmt, va_list args, int width, int x, int title_align, int title_int, wchar_t **wtitle_out, int *title_width_out,
 int *title_start_out)
{
    if (!fmt || !*fmt) return -1;
    int max_title_width = width - 6;
    int title_block_len = 0;
    if (prepare_frame_title(fmt, args, max_title_width, L"..", 0, 0, wtitle_out, title_width_out, &title_block_len) != 0) {
        return -1;
    }
    *title_start_out = calculate_title_start(x, width, title_block_len, title_align, title_int);
    return 0;
}

void generic_frame_ncurses(WINDOW *win, int x, int y, int width, int height, int attr, int fg, int bg, int title_align, int title_int, FrameStyleType style_type,
 const char *fmt, ...)
{
    wattrset(win, A_NORMAL);
    if (width < 3 || height < 2) return;
    const FrameStyle *style = get_frame_style(style_type);
    wattron(win, convert_attr_to_ncurses(attr));
    wattron(win, COLOR_PAIR(get_color_pair(fg, bg)));
    mvwprintw(win, y, x, "%s", style->top_left);
    for (int i = 0; i < width - 2; i++) {
        mvwprintw(win, y, x + 1 + i, "%s", style->horizontal);
    }
    mvwprintw(win, y, x + width - 1, "%s", style->top_right);
    if (fmt && *fmt) {
        va_list args;
        va_start(args, fmt);
        wchar_t *wtitle = NULL;
        int title_width = 0;
        int title_start = 0;
        if (prepare_frame_title_common(fmt, args, width, x, title_align, title_int,
                                      &wtitle, &title_width, &title_start) == 0) {
            mvwprintw(win, y, title_start, "%s ", style->title_left);
            mvwaddnwstr(win, y, title_start + 2, wtitle, (int)wcslen(wtitle));
            mvwprintw(win, y, title_start + 2 + title_width, " %s", style->title_right);
            free(wtitle);
        }
        va_end(args);
    }
    for (int i = 0; i < height - 2; i++) {
        mvwprintw(win, y + 1 + i, x, "%s", style->vertical);
        mvwprintw(win, y + 1 + i, x + width - 1, "%s", style->vertical);
    }
    int bottom_y = y + height - 1;
        mvwprintw(win, bottom_y, x, "%s", style->bottom_left);
    for (int i = 0; i < width - 2; i++) {
        mvwprintw(win, bottom_y, x + 1 + i, "%s", style->horizontal);
    }
        mvwprintw(win, bottom_y, x + width - 1, "%s", style->bottom_right);
        wattroff(win, COLOR_PAIR(get_color_pair(fg, bg)));
        wattroff(win, convert_attr_to_ncurses(attr));
}

void module_strip_ncurses(WINDOW *win, int x, int y, int attr, int fg, int bg, char symbol, const char *module_name) {
    wattrset(win, A_NORMAL);
    char sym = symbol;
    int pos;
    get_module_pos(&sym, &pos);
    int pair = get_color_pair(fg, bg);
    attr_t attr_flag = convert_attr_to_ncurses(attr);
    wmove(win, y - 1, x - 1);
    wattron(win, attr_flag | COLOR_PAIR(pair));
    wprintw(win, "____");
    for (int i = 0; i < 36; i++) {
        if (i == pos) {
            wprintw(win, "[");
            wattroff(win, COLOR_PAIR(pair));
            wattrset(win, A_NORMAL);
            wprintw(win, "%c", sym);
            wattron(win, attr_flag | COLOR_PAIR(pair));
            wprintw(win, "]");
        } else {
            wprintw(win, "[]");
        }
    }
    wprintw(win, "_____");
    wmove(win, y, x + 4 - 1);
    wprintw(win, "%s", module_name);
    wattroff(win, attr_flag | COLOR_PAIR(pair));
    wattrset(win, A_NORMAL);
    wnoutrefresh(win);
    doupdate();
}

void generic_frame_ansi(
    int mod, int x, int y, int width, int height, int attr, int fg, int bg, int title_align, int title_int, FrameStyleType style_type, const char *fmt, ...)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (x < 1) x = 1;
    if (y < 1) y = 1;
    if (x == 1 && y == 1) {
    }
    const FrameStyle *style = get_frame_style(style_type);
    attr_ansi(attr);
    mod_fg_ansi(mod, fg);
    mod_bg_ansi(mod, bg);
    x_y_ansi(x, y);
    printf("%s", style->top_left);
    for (int i = 1; i < width - 1; i++)
        printf("%s", style->horizontal);
    printf("%s", style->top_right);
    printf("\n");
    if (fmt && fmt[0]) {
        va_list args;
        va_start(args, fmt);
        wchar_t *wtitle = NULL;
        int title_width = 0;
        int title_start = 0;
        if (prepare_frame_title_common(fmt, args, width, x, title_align, title_int,
                                      &wtitle, &title_width, &title_start) == 0) {
            x_y_ansi(title_start, y);
            printf("%s ", style->title_left);
            char display_title[512];
            wcstombs(display_title, wtitle, sizeof(display_title));
            printf("%s", display_title);
            printf(" %s", style->title_right);
            free(wtitle);
        }
        va_end(args);
    }
    for (int row = y + 1; row < y + height - 1; row++) {
        if (row > w.ws_row) break;
        x_y_ansi(x, row);
        printf("%s", style->vertical);
        for (int i = 0; i < width - 2; i++) printf(" ");
        printf("%s", style->vertical);
    }
    int bottom_y = y + height - 1;
    if (bottom_y <= w.ws_row) {
        x_y_ansi(x, bottom_y);
        printf("%s", style->bottom_left);
        for (int i = 1; i < width - 1; i++)
            printf("%s", style->horizontal);
        printf("%s", style->bottom_right);
    }
    int scroll_top    = y + 1;
    int scroll_bottom = y + height - 2;
    if (scroll_top < scroll_bottom && scroll_bottom <= w.ws_row) {
        printf("\033[%d;%dr", scroll_top, scroll_bottom);
    }
    x_y_ansi(x + 1, y + 1);
    reset_ansi(1);
}

void module_strip_ansi(int mod, int x, int y, int attr, int fg, int bg, char symbol, const char *module_name) {
    char sym = symbol;
    int pos;
    get_module_pos(&sym, &pos);
    x_y_ansi(x, y);
    attr_ansi(attr);
    mod_fg_ansi(mod, fg);
    mod_bg_ansi(mod, bg);
    printf("\033[1C____");
    for (int i = 0; i < 36; i++) {
        if (i == pos) {
            printf("[");
            printf("\033[0m%c", sym);
            attr_ansi(attr);
            mod_fg_ansi(mod, fg);
            mod_bg_ansi(mod, bg);
            printf("]");
        } else {
            printf("[]");
        }
    }
    printf("_____");
    reset_ansi(1);
    x_y_ansi(x + 1, y + 1);
    printf("    %s", module_name);
    reset_ansi(1);
}
