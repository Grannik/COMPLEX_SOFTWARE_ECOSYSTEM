#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

void group_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_7_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
int handle_common_input(char input_char, int *current_module);

int module_04_run(void);
int module_04_loop(void);

int module_04_loop(void)
{
    int start_y = termheight / 2 - 5;
    while (1) {
        ClockData cd;
        clock_update(&cd);
        var_abc_7_ncurses(stdscr, start_y + 1, 0, 5, -1,
            10, cd.hour_str,
            35, cd.min_str,
            58, cd.sec_str,
            -1);
        int ch = getch();
        flushinp();
        if (ch != ERR) {
            int new_module = 4;
            int result = handle_common_input((char)ch, &new_module);
            if (result == -1) {
                return -1;
            }
            if (new_module != 4) {
                return new_module;
            }
        }
        sleep(1);
    }
    return 4;
}

int module_04_run(void) {
    nodelay(stdscr, TRUE);
    werase(stdscr);
    generic_frame_ncurses(stdscr, 0, 0, 84, termheight, 0, 2, -1, 1, 0, 1, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, '4', "Clock (7-line pseudographic font)");
    int start_y = termheight / 2 - 5;
    group_abc_7_ncurses(stdscr, start_y + 1, 0, 8, -1,
        29, ":",
        52, ":",
        -1);
    return module_04_loop();
}
