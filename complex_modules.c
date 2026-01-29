#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "common.h"
//#include <libmsgtermcolor.h>
#include "libtermcolor/libtermcolor.h"

static int check_terminal_size(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        libtermcolor_msg(37, "WARNING:", 35, " ioctl failed for terminal size check", NUL);
        return 0;
    }
    if (w.ws_col < 84 || w.ws_row < 21) {
        libtermcolor_buf(84, 21, BUF);
        libtermcolor_msg(1, " ", 31, "FATAL:", 22, " ",
                       32, "Terminal too small! Minimum: %dx%d", NUL);
        libtermcolor_buf(w.ws_col, w.ws_row, BUF);
        libtermcolor_msg(1, " ", 33, "WARNING:", 22, " ",
                       35, "Current size: %dx%d", NUL);
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
DECLARE_MODULE_RUN(08);

typedef struct {
    int module_id;
    int current_line;
    int block_size;
    int total;
    int (*run)(void);
} ScrollableModule;

static ScrollableModule scrollable_modules[] = {
{0, 0, 0, 36,  module_00_run},
{5, 0, 0, 59,  module_05_run},
{6, 0, 0, 256, module_06_run},
{7, 0, 0, 256, module_07_run},
{8, 0, 0, 39,  module_08_run},
};

void universal_scroll(int module_id, int direction) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            common_scroll(direction, &scrollable_modules[i].current_line, scrollable_modules[i].block_size, scrollable_modules[i].total);
            break;
        }
    }
}

int universal_get_current_line(int module_id) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            return scrollable_modules[i].current_line;
        }
    }
    return 0;
}

void universal_set_current_line(int module_id, int line) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            if (line < 0) line = 0;
            if (line > scrollable_modules[i].total - scrollable_modules[i].block_size) line = scrollable_modules[i].total - scrollable_modules[i].block_size;
            scrollable_modules[i].current_line = line;
            break;
        }
    }
}

int universal_get_block_size(int module_id) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            return scrollable_modules[i].block_size;
        }
    }
    return 0;
}

void universal_set_block_size(int module_id, int size) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            scrollable_modules[i].block_size = size;
            break;
        }
    }
}

int universal_get_total(int module_id) {
    for (size_t i = 0; i < sizeof(scrollable_modules) / sizeof(scrollable_modules[0]); i++) {
        if (scrollable_modules[i].module_id == module_id) {
            return scrollable_modules[i].total;
        }
    }
    return 0;
}

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
    return 0;
}

int (*module_runs[9])(void) =
{module_00_run, module_01_run, module_02_run, module_03_run, module_04_run, module_05_run, module_06_run, module_07_run, module_08_run};

static int handle_common_input(char ch, int *current_module) {
    if (ch == 'Q') {
        return -1;
    }
    else if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z')) {
        if (ch >= '0' && ch <= '9') {
            *current_module = ch - '0';
        } else {
            *current_module = 10 + (ch - 'a');
        }
        screen_state(0);
        return 1;
    }
    return 0;
}

static int handle_static_input(int *current_module) {
    char ch;
    ssize_t bytes = read(STDIN_FILENO, &ch, 1);
    if (bytes != 1) return 0;
    int result = handle_common_input(ch, current_module);
    if (result != 0) return result;
    if (ch == 27) {
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
                                   universal_scroll(*current_module, -1);
                                   scrollable_modules[i].run();
                                } else if (seq[1] == 'B') {
                                   universal_scroll(*current_module, 1);
                                   scrollable_modules[i].run();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
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
            return handle_common_input(ch, current_module);
        }
    }
    return 0;
}

static int handle_static_input(int *current_module);
static int handle_animated_input(int *current_module);

static int run_main_loop(int start_module) {
    int current_module = start_module;
    while (1) {
        if (current_module >= 0 && current_module <= 8) {
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
                printf("\033[91m Module:\033[0m %d\033[31m empty\033[0m\n", current_module);
                int result = handle_static_input(&current_module);
                if (result == -1) break;
            }
        } else if (current_module >= 8 && current_module <= 35) {
            printf("\033[91m Module:\033[0m ");
            if (current_module <= 9) {
                printf("%d", current_module);
            } else {
                printf("%c", 'a' + (current_module - 10));
            }
            printf("\033[31m empty\033[0m\n");
            int result = handle_static_input(&current_module);
            if (result == -1) break;
        } else {
            printf("\033[91mInvalid module:\033[0m %d\n", current_module);
            current_module = 0;
        }
    }
    return 0;
}

int main(void) {
    if (!check_terminal_size()) {
        return 1;
    }
    int current_module = 0;
    screen_state(0);
    term_mode(0);
    run_main_loop(current_module);
    term_mode(1);
    screen_state(1);
    setbuf(stdout, NULL);
    libtermcolor_msg(37," END", NUL);
    return 0;
}
