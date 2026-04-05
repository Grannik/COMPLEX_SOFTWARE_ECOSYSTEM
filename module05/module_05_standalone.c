#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

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
        if (ch != ERR) {
            if (ch == 'q') {
                terminal_control(0);
                return -1;
            }
        }
        sleep(1);
    }
    return 5;
}

int main(void) {
    terminal_control(1);
    nodelay(stdscr, TRUE);
    werase(stdscr);
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

// clear;gcc module_05_standalone.c ../common.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermanimation/libtermanimation.c ../content_var/content_var.c ../libtermcontrol/libtermcontrol.c -o module_clock_9 -I../ -I../libtermcolor -I../libtermanimation -I../content_var -I../libtermcontrol -lncursesw
