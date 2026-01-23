#include "common.h"
#include "content_color_16.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int module_05_run(void) {
    current_content_size = TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    int total = TOTAL_CONTENT;
    int new_block_size = (available_lines >= total) ? total : available_lines;
    universal_set_block_size(5, new_block_size);
    universal_set_current_line(5, universal_get_current_line(5));
    content_current_line = universal_get_current_line(5);
    content_block_size = new_block_size;
    draw_exact_frame();
    libtermcolor_pos(2, 2, POS, 32,"____[][][][][][", 37,"5", 32,"][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", NUL);
    libtermcolor_pos(6, 3, POS, 32,"Basic ANSI Text Attributes and 16-Color Palette List", NUL);
    display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)content, TOTAL_CONTENT);
    return 0;
}

#ifndef COMPLEX_BUILD

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
        printf("\033[37mWARNING:\033[35m ioctl failed for terminal size check\033[0m\n");
        return 0;
    }
    if (w.ws_col < 84 || w.ws_row < 21) {
        printf("\033[1;31mFATAL: Terminal too small! Minimum: 84x21\033[0m\n");
        printf("\033[33mWARNING: Current size: %dx%d\033[0m\n", w.ws_col, w.ws_row);
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
    libtermcolor_pos(6, 2, POS, 32,"Basic ANSI Text Attributes and 16-Color Palette List", NUL);
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

// gcc -Wall -Wextra -O2 -std=c99 -o module05 module_05.c common.c libtermcolor/libtermcolor.c -I libtermcolor
