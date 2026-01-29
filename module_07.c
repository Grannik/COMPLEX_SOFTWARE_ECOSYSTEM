#include "common.h"
#include "content_color_4_256.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int module_07_run(void) {
    current_content_size = TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    int total = TOTAL_CONTENT;
    int new_block_size = (available_lines >= total) ? total : available_lines;
    universal_set_block_size(7, new_block_size);
    universal_set_current_line(7, universal_get_current_line(7));
    content_current_line = universal_get_current_line(7);
    content_block_size = new_block_size;
    draw_exact_frame();
    libtermcolor_pos(2, 2, POS, 32,"____[][][][][][][][", 37,"7", 32,"][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", NUL);
    libtermcolor_pos(6, 3, POS, 32,"256-Color ANSI Palette List \\033[48;5;*m", NUL);
    display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)content, TOTAL_CONTENT);
    return 0;
}

#ifndef COMPLEX_BUILD

#include <sys/ioctl.h>

void universal_set_block_size(int module_id, int size) {
    (void)module_id; (void)size;
}

int universal_get_current_line(int module_id) {
    (void)module_id;
    return 0;
}

void universal_set_current_line(int module_id, int line) {
    (void)module_id; (void)line;
}

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

int main(void) {
    if (!check_terminal_size()) {
        return 1;
    }
    screen_state(0);
    term_mode(0);
    printf("\033[2J\033[H");
    draw_exact_frame();
    libtermcolor_pos(6, 2, POS, 32,"256-Color ANSI Palette List \033[48;5;*m", NUL);
    current_content = content;
    current_content_size = TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    if (available_lines >= TOTAL_CONTENT) {
        content_block_size = TOTAL_CONTENT;
    } else {
        content_block_size = available_lines;
    }
    content_current_line = 0;
    display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)content, TOTAL_CONTENT);
    while (1) {
        char ch;
        ssize_t bytes = read(STDIN_FILENO, &ch, 1);
        if (bytes == 1) {
            if (ch == 'q' || ch == 'Q') break;
            if (ch == 27) {
                char seq[2];
                bytes = read(STDIN_FILENO, seq, 2);
                if (bytes == 2 && seq[0] == '[') {
                    if (seq[1] == 'A') {
                        if (content_current_line > 0) {
                            content_current_line--;
                            display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)content, TOTAL_CONTENT);
                        }
                    } else if (seq[1] == 'B') {
                        if (content_current_line < TOTAL_CONTENT - content_block_size) {
                            content_current_line++;
                            display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)content, TOTAL_CONTENT);
                        }
                    }
                }
            }
        }
    }
    term_mode(1);
    screen_state(1);
    return 0;
}

#endif

// gcc -Wall -Wextra -O2 -std=c99 -o module06 module_06.c common.c libtermcolor/libtermcolor.c -I libtermcolor
