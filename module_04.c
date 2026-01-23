#include "common.h"
#include "pseudographic_font_7.h"
#include <time.h>
#include <string.h>
#include <unistd.h>

int module_04_run(void) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("\033[2J\033[H");
    draw_exact_frame();
    libtermcolor_pos(2, 2, POS, 32,"____[][][][][", 37,"4", 32,"][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", NUL);
    libtermcolor_pos(6, 3, POS, 32,"Clock (7-line pseudographic font)", NUL);
    print_pseudographic_time(timeinfo->tm_hour,
                             timeinfo->tm_min,
                             timeinfo->tm_sec,
                             get_pseudographic_char_7, 8, 5, 10);
    sleep(1);
    return 0;
}

#ifndef COMPLEX_BUILD
int main(void) {
    screen_state(0);
    term_mode(0);
    while (1) {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        printf("\033[2J\033[H");
        print_pseudographic_time(timeinfo->tm_hour,
                                 timeinfo->tm_min,
                                 timeinfo->tm_sec,
                                 get_pseudographic_char_7, 8, 5, 10);
        fflush(stdout);
        struct timeval tv = {0, 50000};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) == 1) {
                if (ch == 'q' || ch == 'Q') break;
                if (ch >= '0' && ch <= '9') break;
            }
        }
        sleep(1);
    }
    term_mode(1);
    screen_state(1);
    return 0;
}
#endif

// gcc -Wall -Wextra -O2 -std=c99 -o module04 module_04.c common.c libtermcolor/libtermcolor.c -I libtermcolor
