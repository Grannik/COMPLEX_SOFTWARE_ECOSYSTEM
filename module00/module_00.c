#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ncursesw/ncurses.h>

#include "common.h"
#include "content_help.h"
#include "libtermcolor/libtermcolor.h"

#define MENU_WIDTH 78
#define MAX_ITEMS 40

typedef struct {
    const char *description;
    const char *header;
    const char *line_a;
    const char *line_b;
    const char *line_c;
    const char *line_d;
    const char *line_e;
    const char *line_f;
    const char *line_g;
    const char *line_h;
    const char *line_i;
    const char *line_j;
    const char *line_k;
    const char *line_l;
    const char *line_m;
    const char *line_n;
    const char *line_o;
} MenuItem;

static int selected = 0;
static int offset = 0;
static MenuItem items[MAX_ITEMS];

static void init_menu_items(void);
static void print_menu_items(void);
static void display_details(void);
void frame_module(void);
int handle_module_00_input(int ch, int visible);
int module_00_run(void);

static void init_menu_items(void)
{
    for (int i = 0; i < HELP_TOTAL_CONTENT && i < MAX_ITEMS; i++) {
        items[i].description = help_content[i][ 0];
        items[i].header      = help_content[i][ 1];
        items[i].line_a      = help_content[i][ 2];
        items[i].line_b      = help_content[i][ 3];
        items[i].line_c      = help_content[i][ 4];
        items[i].line_d      = help_content[i][ 5];
        items[i].line_e      = help_content[i][ 6];
        items[i].line_f      = help_content[i][ 7];
        items[i].line_g      = help_content[i][ 8];
        items[i].line_h      = help_content[i][ 9];
        items[i].line_i      = help_content[i][10];
        items[i].line_j      = help_content[i][11];
        items[i].line_k      = help_content[i][12];
        items[i].line_l      = help_content[i][13];
        items[i].line_m      = help_content[i][14];
        items[i].line_n      = help_content[i][15];
        items[i].line_o      = help_content[i][16];
    }
    for (int i = HELP_TOTAL_CONTENT; i < MAX_ITEMS; i++) {
        items[i].description = "";
        items[i].header = "";
        items[i].line_a = "";
        items[i].line_b = "";
        items[i].line_c = "";
        items[i].line_d = "";
        items[i].line_e = "";
        items[i].line_f = "";
        items[i].line_g = "";
        items[i].line_h = "";
        items[i].line_i = "";
        items[i].line_j = "";
        items[i].line_k = "";
        items[i].line_l = "";
        items[i].line_m = "";
        items[i].line_n = "";
        items[i].line_o = "";
    }
}

static void print_menu_line(int y, int idx, int is_selected)
{
    if (is_selected) {
        for (int x = 3; x < 3 + MENU_WIDTH - 1; x++) {
            generic_cycle_ncurses(x, y, 0, 2, -1, "%lc", L'▒');
        }
        generic_cycle_ncurses(5, y, 0, 2, -1, "%s", items[idx].description);
    }
    else {
        generic_cycle_ncurses(5, y, 0, 7, -1, "%s", items[idx].description);
    }
}

static void print_menu_items(void)
{
    int start_y = 4;
    int visible = termheight - 8;
    if (visible > 36) visible = 36;
    if (visible < 15) visible = 15;
    int total_items = HELP_TOTAL_CONTENT;
    if (visible >= total_items) {
        offset = 0;
    }
    for (int i = 0; i < visible; i++) {
        mvprintw(start_y + i, 2, "%*s", MENU_WIDTH, "");
    }
    for (int i = 0; i < visible; i++) {
        int idx = offset + i;
        if (idx >= total_items) break;
        if (items[idx].description[0] == '\0') break;
        print_menu_line(start_y + i, idx, (idx == selected));
    }
    refresh();
}

static void display_details(void)
{
    generic_msg_ncurses(6,  5, 1, 7, -1, "%s", items[selected].header);
    generic_msg_ncurses(3,  6, 0, 7, -1, "%s", items[selected].line_a);
    generic_msg_ncurses(3,  7, 0, 7, -1, "%s", items[selected].line_b);
    generic_msg_ncurses(3,  8, 0, 7, -1, "%s", items[selected].line_c);
    generic_msg_ncurses(3,  9, 0, 7, -1, "%s", items[selected].line_d);
    generic_msg_ncurses(3, 10, 0, 7, -1, "%s", items[selected].line_e);
    generic_msg_ncurses(3, 11, 0, 7, -1, "%s", items[selected].line_f);
    generic_msg_ncurses(3, 12, 0, 7, -1, "%s", items[selected].line_g);
    generic_msg_ncurses(3, 13, 0, 7, -1, "%s", items[selected].line_h);
    generic_msg_ncurses(3, 14, 0, 7, -1, "%s", items[selected].line_i);
    generic_msg_ncurses(3, 15, 0, 7, -1, "%s", items[selected].line_j);
    generic_msg_ncurses(3, 16, 0, 7, -1, "%s", items[selected].line_k);
    generic_msg_ncurses(3, 17, 0, 7, -1, "%s", items[selected].line_l);
    generic_msg_ncurses(3, 18, 0, 7, -1, "%s", items[selected].line_m);
    generic_msg_ncurses(3, 19, 0, 7, -1, "%s", items[selected].line_n);
    generic_msg_ncurses(3, 20, 0, 7, -1, "%s", items[selected].line_o);
    getch();
}

void frame_module(void) {
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '0', "Reference information. Module - usage");
}

int handle_module_00_input(int ch, int visible)
{
    int action = handle_module_switch_key(ch);
    if (action == -1 || action > 0) {
        ungetch(ch);
        return 0;
    }
    switch (ch) {
        case KEY_UP:
            selected--;
            if (selected < 0) {
                selected = HELP_TOTAL_CONTENT - 1;
                offset = selected - visible + 1;
                if (offset < 0) offset = 0;
            }
            if (selected < offset) offset = selected;
            break;
        case KEY_DOWN:
            selected++;
            if (selected >= HELP_TOTAL_CONTENT) {
                selected = 0;
                offset = 0;
            }
            if (selected >= offset + visible) {
                offset = selected - visible + 1;
            }
            break;
        case 10:
            clear();
            refresh();
            frame_module();
            display_details();
            break;
        default:
            return 1;
    }
    print_menu_items();
    return 1;
}

int module_00_run(void)
{
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_menu_items();
    frame_module();
    print_menu_items();
    while (1) {
        int ch = getch();
        int visible = termheight - 8;
        if (visible > 36) visible = 36;
        if (visible < 15) visible = 15;
        if (!handle_module_00_input(ch, visible)) {
            break;
        }
    }
    apply_attr_curs(-1);
    return 0;
}
