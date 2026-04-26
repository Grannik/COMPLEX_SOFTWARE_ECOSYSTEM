#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"
#include "../libtermcontrol/libtermcontrol.h"

void group_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_5_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);

int module_03_loop(void)
{
    int start_y = termheight / 2 - 5;
    while (1) {
        ClockData cd;
        clock_update(&cd);
        var_abc_5_ncurses(stdscr, start_y + 1, 0, 5, -1,
            12, "%d", cd.mday,
            31, "%s", cd.hour_str,
            47, "%s", cd.min_str,
            63, "%s", cd.sec_str,
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
    int start_y = termheight / 2 - 5;
    group_abc_5_ncurses(stdscr, start_y + 1, 0, 8, -1,
        25, "/",
        43, ":",
        58, ":",
        -1);
    return module_03_loop();
}

// clear;gcc module_03_standalone.c ../common.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermanimation/libtermanimation.c ../libline/libline.c ../libstrprepare/libstrprepare.c ../libtermcontrol/libtermcontrol.c -o module_clock_5 -I../ -I../libtermcolor -I../libtermanimation -I../libline -I../libstrprepare -I../libtermcontrol -lncursesw
