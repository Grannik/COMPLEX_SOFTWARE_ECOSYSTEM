#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
int module_03_loop(void);

int module_03_loop(void)
{
    while (1) {
        ClockData cd;
        clock_update(&cd);
var_abc_5_ncurses(stdscr, 3, 0, 5, -1,
    10, "%d", cd.mday,
    29, "%s", cd.hour_str,
    45, "%s", cd.min_str,
    61, "%s", cd.sec_str,
    -1);
        int ch = getch();
        if (ch != ERR) {
            if (ch == 'q') {
                terminal_control(0);
                return -1;
            }
        }
        sleep(1);
    }
    return 3;
}

int main(void) {
    terminal_control(1);
    nodelay(stdscr, TRUE);
    werase(stdscr);
    group_abc_5_ncurses(stdscr, 3, 0, 8, -1,
        23, "/",
        40, ":",
        56, ":",
        -1);
    return module_03_loop();
}

// clear;gcc module_03_standalone.c ../common.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermanimation/libtermanimation.c ../content_var/content_var.c ../libtermcontrol/libtermcontrol.c -o module_clock_5 -I../ -I../libtermcolor -I../libtermanimation -I../content_var -I../libtermcontrol -lncursesw
