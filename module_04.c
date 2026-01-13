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
    print_subtitle_left("____[][][][][\033[0m4\033[32m][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", 2, 2);
    print_subtitle_left("Clock (7-line pseudographic font)", 3, 6);
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
    run_animated_module(module_04_run);
    term_mode(1);
    screen_state(1);
    return 0;
}
#endif

// Compilation: gcc -Wall -Wextra -O2 -std=c99 -DSTANDALONE_BUILD -o module04 module_04.c common.c
