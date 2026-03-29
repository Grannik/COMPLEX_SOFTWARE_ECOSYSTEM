#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ncursesw/ncurses.h>

#include "common.h"
#include "content_help.h"
#include "libtermcolor/libtermcolor.h"

#define MENU_WIDTH 78
#define VISIBLE_HEIGHT 30
#define MAX_ITEMS 40
#define MAX_INPUT 79

typedef struct {
    const char *description;
    const char *symbol;
    const char *details[12];
    int detail_lines;
} MenuItem;

static int selected = 0;
static int offset = 0;
static MenuItem items[MAX_ITEMS];

static void init_menu_items(void);
static void print_menu_items(void);
static void display_details(void);
int module_00_run(void);

static void init_menu_items(void)
{
    for (int i = 0; i < HELP_TOTAL_CONTENT && i < MAX_ITEMS; i++) {
        items[i].description = help_content[i][0];
        items[i].symbol      = "";
        items[i].details[0]  = help_content[i][1];
        items[i].detail_lines = 1;
    }
    for (int i = HELP_TOTAL_CONTENT; i < MAX_ITEMS; i++) {
        items[i].description = "";
        items[i].symbol = "";
        items[i].detail_lines = 0;
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

int module_00_run(void)
{
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_pair(4, COLOR_GREEN, -1);
    init_menu_items();
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '0', "Reference information. Module - usage");
    print_menu_items();
    while (1) {
    int ch = getch();
    int action = handle_module_switch_key(ch);
        if (action == -1) {
            return -1;
        }
        if (action > 0) {
            ungetch(ch);
            return 0;
        }
    int visible = termheight - 8;
    if (visible > 36) visible = 36;
    if (visible < 15) visible = 15;
    switch (ch) {
        case KEY_UP:
            selected--;
            if (selected < 0) {
                selected = HELP_TOTAL_CONTENT - 1;
                offset = selected - visible + 1;
                if (offset < 0) offset = 0;
            }
            if (selected < offset) offset = selected;
            print_menu_items();
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
            print_menu_items();
            break;
            case 10:
                clear();
                refresh();
                display_details();
                generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
                module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '0', "Reference information. Module - usage");
                print_menu_items();
                break;
        }
    }
    return 0;
}

static void display_details(void)
{
    generic_msg_ncurses(5, 2, 0, 7, -1, "%s", items[selected].details[0]);
    getch();
}
