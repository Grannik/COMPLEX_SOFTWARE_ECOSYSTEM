#include "common.h"
#include "pseudographic_font_3.h"
#include <time.h>
#include <string.h>
#include <unistd.h>

static void print_pseudographic_time_3x3(int hours, int mins, int secs) {
    if (hours < 0 || hours > 23 || mins < 0 || mins > 59 || secs < 0 || secs > 59) {
        printf("\033[H\033[1mInvalid time: %02d:%02d:%02d\033[0m\n", hours, mins, secs);
        fflush(stdout);
        return;
    }

    char time_str[9];
    snprintf(time_str, 9, "%02d:%02d:%02d", hours, mins, secs);

    const char** chars[8];
    for (int i = 0; i < 8; i++) {
        chars[i] = get_pseudographic_char_3(time_str[i]);
    }
    printf("\033[2;1H");
    for (int row = 0; row < 3; row++) {
        printf("  ");
        for (int i = 0; i < 8; i++) {
            if (i == 2 || i == 5) {
                printf("\033[1m\033[90m%s\033[0m", chars[i][row]);
            } else {
                printf("\033[1m\033[37m%s\033[0m", chars[i][row]);
            }
        }
        printf("\n");
    }
    fflush(stdout);
}

int module_02_run(void) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    printf("\033[2J\033[H");
    print_pseudographic_time_3x3(timeinfo->tm_hour,
                                timeinfo->tm_min,
                                timeinfo->tm_sec);
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
