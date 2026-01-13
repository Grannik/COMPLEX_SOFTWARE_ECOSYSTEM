#include "common.h"
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

const char **current_content = NULL;
int current_content_size = 0;
int content_block_size = 0;
int content_current_line = 0;

void screen_state(int flag) {
    if (flag == 0) {
        printf("\033[?25l");
        printf("\033[2J\033[H");
    } else if (flag == 1) {
        printf("\033[2J\033[H");
        printf("\033[?25h");
    }
}

void term_mode(int flag) {
    static struct termios oldt, newt;
    if (flag == 0) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    else if (flag == 1) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

int wait_switch(void) {
    char ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
    if (bytes_read != 1) return -2;
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    else if (ch == 'q' || ch == 'Q') {
        return -1;
    }
    if (ch == 27) {
        char discard[10];
        struct timeval tv = {0, 10000};
        fd_set fds;
        while (1) {
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0) {
                break;
            }
            ssize_t discarded = read(STDIN_FILENO, discard, sizeof(discard));
            if (discarded <= 0) break;
            tv.tv_usec = 1000;
        }
    }
    return -2;
}

void wait_exit(void) {
    char ch;
    while (1) {
        ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
        if (bytes_read == 1 && (ch == 'q' || ch == 'Q')) {
            break;
        }
    }
}

void run_animated_module(int (*module_func)(void)) {
    while (1) {
        module_func();
        struct timeval tv = {0, 50000};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret > 0) {
            char ch;
            ssize_t bytes = read(STDIN_FILENO, &ch, 1);
            if (bytes == 1 && (ch == 'q' || ch == 'Q')) break;
        }
    }
}

void draw_exact_frame() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_width = w.ws_col;
    int term_height = w.ws_row;
    const char *title = "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM";
    int fixed_width = 84;
    int actual_width = (term_width < fixed_width) ? term_width : fixed_width;
    if (actual_width < 4) return;
    printf("\033[38;5;10m");
    printf("\033[2J\033[H");
    printf("╔");
    for (int i = 1; i < actual_width - 1; i++) {
        printf("═");
    }
    printf("╗\n");
    if (title && title[0]) {
        int title_len = strlen(title);
        int title_width = title_len + 4;
        int title_start = (actual_width - title_width) / 2;
        if (title_start < 1) title_start = 1;
        printf("\033[1;%dH╡ %s ╞", title_start + 1, title);
    }
    for (int y = 2; y < term_height; y++) {
        printf("\033[%d;1H║", y);
        printf("\033[%d;%dH║", y, actual_width);
    }
    printf("\033[%d;1H╚", term_height);
    for (int i = 1; i < actual_width - 1; i++) {
        printf("═");
    }
    printf("╝");
    printf("\033[0m\033[2;1H");
    fflush(stdout);
}

int get_terminal_height() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
}

void display_from_line(int start_line) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = (w.ws_col < 84) ? w.ws_col : 84;
    for (int row = 0; row < content_block_size; row++) {
        printf("\033[%d;2H", 4 + row);
        int line_num = start_line + row;
        if (current_content && line_num < current_content_size) {
            printf("\033[K%s", current_content[line_num]);
        } else {
            printf("\033[K");
        }
        printf("\033[38;5;10m\033[%d;%dH║\033[0m", 4 + row, width);
    }
    fflush(stdout);
}

void common_scroll(int direction, int *current_line, int block_size, int total) {
    if (block_size >= total) return;
    int new_line = *current_line + direction;
    if (new_line < 0) new_line = 0;
    if (new_line > total - block_size) new_line = total - block_size;
    *current_line = new_line;
}

void print_subtitle_left(const char *title, int row, int left_margin) {
    printf("\033[%d;%dH\033[38;5;10m%s\033[0m", row, left_margin, title);
    fflush(stdout);
}

void print_pseudographic_time(int hours, int mins, int secs,
                              GetCharFunc get_char_func,
                              int font_rows, int start_row, int start_col) {
    if (hours < 0 || hours > 23 || mins < 0 || mins > 59 || secs < 0 || secs > 59) {
        printf("\033[H\033[1mInvalid time: %02d:%02d:%02d\033[0m\n", hours, mins, secs);
        fflush(stdout);
        return;
    }
    char time_str[9];
    snprintf(time_str, 9, "%02d:%02d:%02d", hours, mins, secs);
    const char** chars[8];
    for (int i = 0; i < 8; i++) {
        chars[i] = get_char_func(time_str[i]);
    }
    for (int row = 0; row < font_rows; row++) {
        printf("\033[%d;%dH", start_row + row, start_col);
        for (int i = 0; i < 8; i++) {
            if (i == 2 || i == 5) {
                printf("\033[1m\033[90m%s\033[0m", chars[i][row]);
            } else {
                printf("\033[1m\033[37m%s\033[0m", chars[i][row]);
            }
        }
    }
    fflush(stdout);
}
