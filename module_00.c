#include "common.h"
#include "content_help.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <string.h>

static int current_line = 0;
static int block_size = 0;

static void scroll_lines(int direction) {
    common_scroll(direction, &current_line, block_size, TOTAL_CONTENT);
}

void module_0_scroll_up(void) {
    scroll_lines(-1);
}

void module_0_scroll_down(void) {
    scroll_lines(1);
}

int module_0_get_current_line(void) {
    return current_line;
}

 void module_0_set_current_line(int line) {
    if (line < 0) line = 0;
    if (line > TOTAL_CONTENT - block_size) line = TOTAL_CONTENT - block_size;
    current_line = line;
}

int module_00_run(void) {
    current_content = content;
    current_content_size = TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    if (available_lines >= TOTAL_CONTENT) {
        block_size = TOTAL_CONTENT;
    } else {
        block_size = available_lines;
    }
    content_block_size = block_size;
    content_current_line = current_line;
    draw_exact_frame();
    print_subtitle_left("____[\033[0m0\033[32m][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", 2, 2);
    print_subtitle_left("Reference information. Module - usage", 3, 6);
    display_from_line(current_line);
    return 0;
}

#ifdef STANDALONE_BUILD

int handle_arrow_input_fixed(void) {
    char ch;
    ssize_t bytes = read(STDIN_FILENO, &ch, 1);
    if (bytes != 1) return -4;
    if (ch != 27) return -4;
    char seq[2];
    bytes = read(STDIN_FILENO, seq, 2);
    if (bytes == 2 && seq[0] == '[') {
        if (seq[1] == 'A') return -2;
        if (seq[1] == 'B') return -3;
    }
    tcflush(STDIN_FILENO, TCIFLUSH);
    return -4;
}

int handle_all_input(void) {
    char ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);
    if (bytes_read != 1) return -4;
    if (ch >= '0' && ch <= '9') {
        tcflush(STDIN_FILENO, TCIFLUSH);
        return ch;
    }
    else if (ch == 'q' || ch == 'Q') {
        tcflush(STDIN_FILENO, TCIFLUSH);
        return -1;
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
                FD_ZERO(&fds);
                FD_SET(STDIN_FILENO, &fds);
                if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                    seq_bytes = read(STDIN_FILENO, &seq[1], 1);
                    if (seq_bytes == 1) {
                        if (seq[1] == 'A') {
                            tcflush(STDIN_FILENO, TCIFLUSH);
                            return -2;
                        }
                        else if (seq[1] == 'B') {
                            tcflush(STDIN_FILENO, TCIFLUSH);
                            return -3;
                        }
                    }
                }
            }
        }
        tcflush(STDIN_FILENO, TCIFLUSH);
        return -4;
    }
    tcflush(STDIN_FILENO, TCIFLUSH);
    return -4;
}

int main(void) {
    screen_state(0);
    term_mode(0);
    current_content = content;
    current_content_size = TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    if (available_lines >= TOTAL_CONTENT) {
        block_size = TOTAL_CONTENT;
    } else {
        block_size = available_lines;
    }
    content_block_size = block_size;
    content_current_line = current_line;
    current_line = 0;
    draw_exact_frame();
    display_from_line(current_line);
    tcflush(STDIN_FILENO, TCIFLUSH);
    int running = 1;
    while (running) {
        int action = handle_all_input();
        if (action == -1) {
            running = 0;
        }
        else if (action == -2) {
            scroll_lines(-1);
            display_from_line(current_line);
        }
        else if (action == -3) {
            scroll_lines(1);
            display_from_line(current_line);
        }
        else if (action >= '0' && action <= '6') {
            running = 0;
        }
    }
    term_mode(1);
    screen_state(1);
    return 0;
}

#endif
// компиляция: gcc -Wall -Wextra -O2 -std=c99 -DSTANDALONE_BUILD -o module00 module_00.c common.c
