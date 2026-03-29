#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

int handle_common_input(char input_char, int *current_module);
int module_05_run(void);
int module_05_loop(void);

int module_05_loop(void)
{
    while (1) {
        ClockData cd;
        clock_update(&cd);
        int maxy, maxx;
        getmaxyx(stdscr, maxy, maxx);
        int y = (maxy / 2) - 4;
        int hour_x = (maxx / 2) - 25;
        int min_x  = (maxx / 2) + 7;
        var_abc_9_ncurses(stdscr, y, 0, 5, -1,
            hour_x,  cd.hour_str,
            min_x,   cd.min_str,
            -1);
        int ch = getch();
        flushinp();
        if (ch != ERR) {
            int new_module = 5;
            int result = handle_common_input((char)ch, &new_module);
            if (result == -1) {
                return -1;
            }
            if (new_module != 5) {
                return new_module;
            }
        }
        sleep(1);
    }
    return 5;
}

int module_05_run(void) {
    nodelay(stdscr, TRUE);
    werase(stdscr);
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 7, '5', "Clock (9-line pseudographic font)");
int maxy, maxx;
getmaxyx(stdscr, maxy, maxx);

generic_abc_9_ncurses(
    stdscr,
    (maxx / 2) - 4,
    (maxy / 2) - 4,
    0, 8, -1,
    ":"
);
    return module_05_loop();
}
