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
#include "../libtermanimation/libtermanimation.h"

int main_loop(void);

int main_loop(void)
{
    while (1) {
    ClockData cd;
    clock_update(&cd);
    var_generic_msg_ncurses(2, -1, 5, -1,
         5, "%d", cd.year,
        12, "%s", cd.season,
        21, "%s", cd.month_name,
        29, "%d", cd.mday,
        32, "%s", cd.weekday_name,
        42, "%02d", cd.hour,
        45, "%02d", cd.min,
        48, "%02d", cd.sec,
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
    return 1;
}

int main(void) {
    terminal_control(1);
    nodelay(stdscr, TRUE);
    werase(stdscr);
    group_msg(2, -1, 8, -1, 10,"/", 19,"/", 27,"|", 31,"/", 40,"|", 44,":", 47,":", -1,(const char*)-1);
    return main_loop();
}
// clear;gcc module_01_standalone.c ../common.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermanimation/libtermanimation.c ../content_var/content_var.c ../libtermcontrol/libtermcontrol.c -o module_clock_1 -I../ -I../libtermcolor -I../libtermanimation -I../content_var -I../libtermcontrol -lncursesw
