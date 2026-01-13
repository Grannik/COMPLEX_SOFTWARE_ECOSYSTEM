#include "common.h"
#include "pseudographic_font_3.h"
#include <time.h>
#include <string.h>
#include <unistd.h>

int module_02_run(void) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("\033[2J\033[H");
    draw_exact_frame();
    print_subtitle_left("____[][][\033[0m2\033[32m][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", 2, 2);
    print_subtitle_left("Clock (3-line pseudographic font)", 3, 6);
    print_pseudographic_time(timeinfo->tm_hour,
                            timeinfo->tm_min,
                            timeinfo->tm_sec,
                            get_pseudographic_char_3, 3, 5, 29);
    sleep(1);
    return 0;
}

#ifndef COMPLEX_BUILD
int main(void) {
    screen_state(0);
    term_mode(0);
    run_animated_module(module_02_run);
    term_mode(1);
    screen_state(1);
    return 0;
}
#endif

// Compilation: gcc -Wall -Wextra -O2 -std=c99 -DSTANDALONE_BUILD -o module02 module_02.c common.c
