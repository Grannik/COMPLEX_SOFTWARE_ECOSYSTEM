#include "common.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "libtermcolor/libtermcolor.h"
#include "libtermanimation/libtermanimation.h"

void group_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
void var_abc_ncurses(WINDOW *win, int y, int attr, int fg, int bg, ...);
int module_02_loop(void);
int get_start_y(void);
int module_02_loop(void)
{
    while (1) {
        ClockData cd;
        clock_update(&cd);
        var_abc_ncurses(stdscr, 2, 1, 5, -1,
             5, "%d",   cd.mday,
            17, "%s",   cd.weekday_name,

            40, "%02d", cd.hour,
            48, "%02d", cd.min,
            56, "%02d", cd.sec,
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
    return 2;
}

int main(void) {
    terminal_control(1);
    nodelay(stdscr, TRUE);
    werase(stdscr);
    group_abc_ncurses(stdscr, 2, 0, 8, -1,
        12, "/",
        45, ":",
        53, ":",
        -1);
    return module_02_loop();
}

// clear;gcc module_02_standalone.c ../common.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermanimation/libtermanimation.c ../content_var/content_var.c ../libtermcontrol/libtermcontrol.c -o module_clock_3 -I../ -I../libtermcolor -I../libtermanimation -I../content_var -I../libtermcontrol -lncursesw
