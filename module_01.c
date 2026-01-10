#include "common.h"
#include <time.h>
#include <string.h>
#include <unistd.h>

static const char* get_season(int month) {
    const char* seasons[] = {"Winter", "Spring", "Summer", "Autumn"};
    if (month == 12 || month <= 2) return seasons[0];
    if (month >= 3 && month <= 5) return seasons[1];
    if (month >= 6 && month <= 8) return seasons[2];
    return seasons[3];
}

static const char* get_month_name(int month) {
    const char* months[] = {"January", "February", "March", "April", "May", "June",
                           "July", "August", "September", "October", "November", "December"};
    return months[month - 1];
}

static const char* get_weekday_name(int weekday) {
    const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday"};
    return weekdays[weekday];
}

int module_01_run(void) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    const char* season = get_season(timeinfo->tm_mon + 1);
    const char* month_name = get_month_name(timeinfo->tm_mon + 1);
    const char* weekday_name = get_weekday_name(timeinfo->tm_wday);
    printf("\033[2J\033[H");
    printf("\033[2;1H");
    printf(" %d\033[90m/\033[0m%s\033[90m/\033[0m%s \033[90;1m|\033[0m %d\033[90m/\033[0m%s \033[90;1m|\033[0m %02d\033[90m:\033[0m%02d\033[90m:\033[0m%02d",
           timeinfo->tm_year + 1900,
           season,
           month_name,
           timeinfo->tm_mday,
           weekday_name,
           timeinfo->tm_hour,
           timeinfo->tm_min,
           timeinfo->tm_sec);
    fflush(stdout);
    sleep(1);
    return 0;
}

#ifndef COMPLEX_BUILD
int main(void) {
    screen_state(0);
    term_mode(0);
    run_animated_module(module_01_run);
    term_mode(1);
    screen_state(1);
    return 0;
}
#endif
