#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include "common.h"

#define DECLARE_MODULE_RUN(num) extern int module_##num##_run(void) __attribute__((weak))

DECLARE_MODULE_RUN(00);
DECLARE_MODULE_RUN(01);
DECLARE_MODULE_RUN(02);
DECLARE_MODULE_RUN(03);
DECLARE_MODULE_RUN(04);
DECLARE_MODULE_RUN(05);
DECLARE_MODULE_RUN(06);

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
    if (module_num == 6) {
        return 1;
    }
    return 0;
}

int main(void) {
    int (*module_runs[7])(void) = {
        module_00_run, module_01_run, module_02_run, module_03_run, module_04_run, module_05_run, module_06_run
    };
    int current_module = 0;
    screen_state(0);
    term_mode(0);
    while (1) {
        if (module_runs[current_module] != NULL) {
            module_runs[current_module]();
            if (is_animated_module(current_module)) {
                fd_set fds;
                struct timeval tv = {0, 50000};
                FD_ZERO(&fds);
                FD_SET(STDIN_FILENO, &fds);
                if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                    char ch;
                    ssize_t bytes = read(STDIN_FILENO, &ch, 1);
                    if (bytes == 1) {
                        if (ch == 'q' || ch == 'Q') break;
                        else if (ch >= '0' && ch <= '6') {
                            current_module = ch - '0';
                            screen_state(0);
                        }
                    }
                }
            } else {
                char ch;
                ssize_t bytes = read(STDIN_FILENO, &ch, 1);
                if (bytes == 1) {
                    if (ch == 'q' || ch == 'Q') {
                        break;
                    }
                    else if (ch >= '0' && ch <= '6') {
                        current_module = ch - '0';
                        screen_state(0);
                    }
                    else if (ch == 27) {
                        if (current_module == 0 || current_module == 5) {
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
                                            if (seq[1] == 'A') {
                                                if (current_module == 0) {
                                                    module_0_scroll_up();
                                                    module_00_run();
                                                } else if (current_module == 5) {
                                                    module_5_scroll_up();
                                                    module_05_run();
                                                }
                                                continue;
                                            }
                                            else if (seq[1] == 'B') {
                                                if (current_module == 0) {
                                                    module_0_scroll_down();
                                                    module_00_run();
                                                } else if (current_module == 5) {
                                                    module_5_scroll_down();
                                                    module_05_run();
                                                }
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            }
        } else {
            printf("\033[1;31mModule %d not available\033[0m\n", current_module);
            break;
        }
    }
    term_mode(1);
    screen_state(1);
    return 0;
}
