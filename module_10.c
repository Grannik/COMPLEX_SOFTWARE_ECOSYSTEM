#include <curses.h>
#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "content_color/content_abc.h"

extern int termheight;
int module_10_run(void);
int module_10_run(void) {
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, 'A', "ABC");
    int content_height = termheight - 4;
    int content_width = 80;
    int visible_lines = content_height;
    static int scroll_offset = 0;
    WINDOW *content_win = derwin(stdscr, content_height, content_width, 4, 2);
    keypad(content_win, TRUE);
    while (1) {
        wattron(content_win, convert_attr_to_ncurses(0));
        wattron(content_win, COLOR_PAIR(get_color_pair(5, -1)));
        display_content_with_scroll(content_win, 0, 0, content_height, content_width, abc_content, ABC_TOTAL_CONTENT, &scroll_offset);
        wattroff(content_win, COLOR_PAIR(get_color_pair(5, -1)));
        wattroff(content_win, convert_attr_to_ncurses(0));
        int ch = wgetch(content_win);
        int action = handle_module_switch_key(ch);
        if (action == -1) {
            delwin(content_win);
            return -1;
        }
        if (action > 0) {
            ungetch(ch);
            delwin(content_win);
            return 0;
        }
        if (ch == KEY_UP || ch == KEY_DOWN) {
            handle_scroll_keys(ch, &scroll_offset, ABC_TOTAL_CONTENT, visible_lines);
        }
    }
}
