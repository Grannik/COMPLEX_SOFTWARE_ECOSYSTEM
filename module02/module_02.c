#include "common.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

void group_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

int handle_common_input(char input_char, int *current_module);

int module_02_draw(void);
int module_02_update(void);

int get_start_y(void);
int get_start_y(void) {
    return termheight / 2 - 5;
}

int module_02_draw(void) {
    int start_y = get_start_y();

    werase(stdscr);
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '2', "Clock (3-line pseudographic font)");
    group_abc_ncurses(stdscr, start_y, 0, 8, -1,
        12, "/", 52, ":", 60, ":", -1);

    ClockData cd;
    clock_update(&cd);
    var_abc_ncurses(stdscr, start_y, 1, 5, -1,
        5, "%d", cd.mday,
        17, "%s", cd.weekday_name,
        47, "%02d", cd.hour,
        55, "%02d", cd.min,
        63, "%02d", cd.sec,
        -1);

    wrefresh(stdscr);
    return 2;
}

int module_02_update(void) {
    int start_y = get_start_y();

    ClockData cd;
    clock_update(&cd);
    var_abc_ncurses(stdscr, start_y, 1, 5, -1,
        5, "%d", cd.mday,
        17, "%s", cd.weekday_name,
        47, "%02d", cd.hour,
        55, "%02d", cd.min,
        63, "%02d", cd.sec,
        -1);

    wrefresh(stdscr);
    return 2;
}
