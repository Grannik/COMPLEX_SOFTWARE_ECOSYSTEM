#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
int module_03_run(void);
int module_03_loop(void);

int module_03_loop(void)
{
    int start_y = termheight / 2 - 5;
    while (1) {
        ClockData cd;
        clock_update(&cd);
var_abc_5_ncurses(stdscr, start_y + 1, 0, 5, -1,
    5, "%d", cd.mday,
    24, "%s", cd.hour_str,
    40, "%s", cd.min_str,
    56, "%s", cd.sec_str,
    -1);
        int ch = getch();
        flushinp();
        if (ch != ERR) {
            int new_module = 3;
            int result = handle_common_input((char)ch, &new_module);
            if (result == -1) {
                return -1;
            }
            if (new_module != 3) {
                return new_module;
            }
        }
        sleep(1);
    }
    return 3;
}

int module_03_run(void) {
    nodelay(stdscr, TRUE);
    werase(stdscr);
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '3', "Clock (5-line pseudographic font)");
    int start_y = termheight / 2 - 5;
    group_abc_5_ncurses(stdscr, start_y + 1, 0, 8, -1,
        18, "/",
        35, ":",
        51, ":",
        -1);
    return module_03_loop();
}
