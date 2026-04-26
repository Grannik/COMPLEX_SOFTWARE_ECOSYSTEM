#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#include "../common.h"
#include "../libtermcolor/libtermcolor.h"
#include "../libline/libline.h"
#include "../libtermanimation/libtermanimation.h"

int handle_common_input(char input_char, int *current_module);
int module_01_draw(void);
int module_01_update(void);

int module_01_draw(void) {
    werase(stdscr);
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, '1', "Clock (1-line pseudographic font)");
    group_msg(5, 1, 8, -1,
        25,"/", 34,"/", 42,"|", 46,"/", 55,"|", 59,":", 62,":",
        -1,(const char*)-1);

    ClockData cd;
    clock_update(&cd);
    var_generic_msg_ncurses(5, 1, 5, -1,
        20, "%d", cd.year,
        28, "%s", cd.season,
        36, "%s", cd.month_name,
        44, "%d", cd.mday,
        47, "%s", cd.weekday_name,
        57, "%02d", cd.hour,
        60, "%02d", cd.min,
        63, "%02d", cd.sec,
        -1);

    wrefresh(stdscr);
    return 1;
}

int module_01_update(void) {
    ClockData cd;
    clock_update(&cd);
    var_generic_msg_ncurses(5, 1, 5, -1,
        20, "%d", cd.year,
        28, "%s", cd.season,
        36, "%s", cd.month_name,
        44, "%d", cd.mday,
        47, "%s", cd.weekday_name,
        57, "%02d", cd.hour,
        60, "%02d", cd.min,
        63, "%02d", cd.sec,
        -1);
    wrefresh(stdscr);
    return 1;
}
