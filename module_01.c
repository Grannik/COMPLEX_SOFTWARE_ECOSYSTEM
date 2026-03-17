#define _XOPEN_SOURCE 500
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

int module_01_run(void);
int handle_common_input(char input_char, int *current_module);
int main_loop(void);

int main_loop(void)
{
    while (1) {
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
        int ch = getch();
        flushinp();
        if (ch != ERR) {
            int new_module = 1;
            int result = handle_common_input((char)ch, &new_module);
            if (result == -1) {
                return -1;
            }
            if (new_module != 1) {
                return new_module;
            }
        }
        sleep(1);
    }
    return 1;
}

int module_01_run(void) {
nodelay(stdscr, TRUE);
    werase(stdscr);
    generic_frame_ncurses(stdscr, 0, 0, 84, termheight, 0, 2, -1, 1, 0, 1, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, '1', "Clock (1-line pseudographic font)");
    group_msg(5, 1, 8, -1,
    25,"/", 34,"/", 42,"|", 46,"/", 55,"|", 59,":", 62,":",
    -1,(const char*)-1);
    return main_loop();
    wnoutrefresh(stdscr);
    doupdate();
    return 0;
}
