#include <curses.h>
#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "content_help.h"

extern int termheight;
int module_00_run(void);
int module_00_run(void) {
    generic_frame_ncurses(stdscr, 0, 0, 84, termheight, 0, 2, -1, 1, 0, 1, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, '0', "Reference information. Module - usage");
    int content_height = termheight - 4;
    int content_width = 80;
    int visible_lines = content_height;
    static int scroll_offset = 0;
    WINDOW *content_win = derwin(stdscr, content_height, content_width, 4, 2);
    keypad(content_win, TRUE);
    int running = 1;
    while (running) {
        display_content_with_scroll(content_win, 0, 0, content_height, content_width,
                                   help_content, HELP_TOTAL_CONTENT, &scroll_offset);
        int ch = wgetch(content_win);
        if (ch == KEY_UP || ch == KEY_DOWN) {
            handle_scroll_keys(ch, &scroll_offset, HELP_TOTAL_CONTENT, visible_lines);
        } else {
            ungetch(ch);
            running = 0;
        }
    }
    delwin(content_win);
    return 0;
}
