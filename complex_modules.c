#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "common.h"

static int check_terminal_size(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return 0;
    }
    if (w.ws_col < 84 || w.ws_row < 21) {
        printf("\033[1;31m Terminal too small! Minimum: 84x21\033[0m\n");
        printf("\033[33m Current: %dx%d\033[0m\n", w.ws_col, w.ws_row);
        return 0;
    }
    return 1;
}

#define DECLARE_MODULE_RUN(num) extern int module_##num##_run(void) __attribute__((weak))

DECLARE_MODULE_RUN(00);
DECLARE_MODULE_RUN(01);
DECLARE_MODULE_RUN(02);
DECLARE_MODULE_RUN(03);
DECLARE_MODULE_RUN(04);
DECLARE_MODULE_RUN(05);
DECLARE_MODULE_RUN(06);
DECLARE_MODULE_RUN(07);

typedef struct {
    int module_id;
    void (*scroll_up)(void);
    void (*scroll_down)(void);
    int (*run)(void);
} ScrollableModule;

static ScrollableModule scrollable_modules[] = {
    {0, module_0_scroll_up,   module_0_scroll_down,   module_00_run},
    {5, module_5_scroll_up,   module_5_scroll_down,   module_05_run},
    {6, module_6_scroll_up,   module_6_scroll_down,   module_06_run},
};

static int is_animated_module(int module_num) {
    if (module_num == 1) {
        return 1;
    }
    if (module_num == 2) {
        return 1;
    }
    if (module_num == 3) {
        return 1;
    }
    if (module_num == 4) {
        return 1;
    }
    if (module_num == 7) {
        return 1;
    }
    return 0;
}

    int (*module_runs[8])(void) = {
        module_00_run, module_01_run, module_02_run, module_03_run, module_04_run, module_05_run,
        module_06_run, module_07_run
    };

static int handle_static_input(int *current_module) {
    char ch;
    ssize_t bytes = read(STDIN_FILENO, &ch, 1);
    if (bytes != 1) return 0;

    if (ch == 'q' || ch == 'Q') {
        return -1;
    }
    else if (ch >= '0' && ch <= '7') {
        *current_module = ch - '0';
        screen_state(0);
        return 1;
    }
    else if (ch == 27) {
        char seq[2];
        struct timeval tv = {0, 10000};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            ssize_t seq_bytes = read(STDIN_FILENO, &seq[0], 1);
            if (seq_bytes == 1 && seq[0] == '[') {
                if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                    seq_bytes = read(STDIN_FILENO, &seq[1], 1);
                    if (seq_bytes == 1) {
                        for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
                            if (*current_module == scrollable_modules[i].module_id) {
                                if (seq[1] == 'A') {
                                    scrollable_modules[i].scroll_up();
                                    scrollable_modules[i].run();
                                } else if (seq[1] == 'B') {
                                    scrollable_modules[i].scroll_down();
                                    scrollable_modules[i].run();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    return 0;
}

static int handle_animated_input(int *current_module) {
    fd_set fds;
    struct timeval tv = {0, 50000};
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
        char ch;
        ssize_t bytes = read(STDIN_FILENO, &ch, 1);
        if (bytes == 1) {
            if (ch == 'q' || ch == 'Q') {
                return -1;
            }
            else if (ch >= '0' && ch <= '7') {
                *current_module = ch - '0';
                screen_state(0);
                return 1;
            }
        }
    }
    return 0;
}

static int handle_static_input(int *current_module);
static int handle_animated_input(int *current_module);

int main(void) {
    if (!check_terminal_size()) {
        return 1;
    }
    int current_module = 0;
    screen_state(0);
    term_mode(0);
    while (1) {
        if (module_runs[current_module] != NULL) {
            module_runs[current_module]();
            int result;
            if (is_animated_module(current_module)) {
                result = handle_animated_input(&current_module);
            } else {
                result = handle_static_input(&current_module);
            }
            if (result == -1) break;
        } else {
            printf("\033[1;31mModule %d not available\033[0m\n", current_module);
            break;
        }
    }
    term_mode(1);
    screen_state(1);
    return 0;
}
