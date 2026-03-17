#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <curses.h>

#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"

#define MIN_WIDTH 84
#define MIN_HEIGHT 21

int termheight = 0;
int termwidth = 0;
int content_block_size = 0;
int content_current_line = 0;

int terminal_check_size(int min_width, int min_height)
{
    if (isatty(STDOUT_FILENO)) {
        int rows, cols;
        term_screen_get_size(&rows, &cols);
        if (cols < min_width || rows < min_height) {
            reset_ansi(2);
            generic_msg_ansi(16, 1, 1, 1, 31, -1, " FATAL:");
            generic_msg_ansi(
                16, 9, 1, 3, 35, -1,
                "Terminal too small! Minimum: %dx%d",
                min_width, min_height
            );
            generic_msg_ansi(16, 1, 2, 1, 31, -1, " WARNING:");
            generic_msg_ansi(
                16, 11, 2, 3, 35, -1,
                "Current size: %dx%d",
                cols, rows
            );
            return -1;
        }
    }
    return 0;
}

int terminal_control(int mode)
{
    static int colors_initialized = 0;
    static struct termios orig_termios;
    if (mode != 1 && mode != 0) {
        reset_ansi(2);
        generic_msg_ansi(16, 1, 1, 1, 31, -1, " ERROR:");
        generic_msg_ansi(16, 8, 1, 3, 35, -1, "terminal_control(): invalid argument '%d'", mode);
        generic_msg_ansi(16, 1, 2, 1, 31, -1, " USAGE:");
        generic_msg_ansi(16, 8, 2, 3, 35, -1, "TERMINAL_MODE_SETUP (0) or TERMINAL_MODE_RESTORE (1)");
        return -1;
    }
    switch (mode) {
    case 1: {
        if (colors_initialized) {
            reset_ansi(2);
            generic_msg_ansi(16, 1, 2, 1, 31, -1, " WARNING:");
            generic_msg_ansi(16, 11, 2, 3, 35, -1, "terminal already initialized, skipping setup");
            return 0;
        }
        if (terminal_check_size(84, 21) == -1) {
            return -1;
        }
        if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        }
        if (setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
            setlocale(LC_ALL, "");
        }
        setlocale(LC_CTYPE, "");

        if (initscr() == NULL) {
            reset_ansi(2);
            generic_msg_ansi(16, 1, 1, 1, 31, -1, " FATAL:");
            generic_msg_ansi(16, 9, 1, 3, 35, -1, "ncurses initialization failed");
            return -1;
        }
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        idlok(stdscr, TRUE);
        scrollok(stdscr, FALSE);
        getmaxyx(stdscr, termheight, termwidth);
        curs_set(0);
        colors_initialized = 1;
        return 0;
    }
    case 0:
        if (!colors_initialized) {
            curs_set(1);
            echo();
            nocbreak();
            write(STDOUT_FILENO, "\033[?25h\033[0m", 10);
            return 0;
        }
        curs_set(1);
        echo();
        nocbreak();
        if (colors_initialized) {
            endwin();
        }
        write(STDOUT_FILENO, "\033[3J\033[2J\033[H", 11);
        fflush(stdout);
        colors_initialized = 0;
        generic_msg_ansi(16, 1, 1, 0, 36, -1, " END");
        return 0;
    default:
        return -1;
    }
}

void display_content_with_scroll(WINDOW *win, int start_y, int start_x, int height, int width, const char **content, int total_lines, int *scroll_offset) {
    if (!win || !content || !scroll_offset) return;
    int visible_lines = height - 2;
    for (int i = 0; i < visible_lines; i++) {
        mvwprintw(win, start_y + i, start_x, "%*s", width - 2, "");
    }
    for (int i = 0; i < visible_lines && (*scroll_offset + i) < total_lines; i++) {
        if (content[*scroll_offset + i]) {
            mvwprintw(win, start_y + i, start_x, "%-*.*s",
                     width - 2, width - 2,
                     content[*scroll_offset + i]);
        }
    }
    wmove(win, 0, 0);
    wrefresh(win);
}

int handle_scroll_keys(int ch, int *scroll_offset, int total_lines, int visible_lines) {
    if (!scroll_offset) return 0;
    switch(ch) {
        case KEY_UP:
            if (*scroll_offset > 0) {
                (*scroll_offset)--;
                return 1;
            }
            break;
        case KEY_DOWN:
            if (*scroll_offset < total_lines - visible_lines) {
                (*scroll_offset)++;
                return 1;
            }
            break;
    }
    return 0;
}

 void display_from_line(int start_line, void *content, int content_size)
{
    const char **lines = (const char **)content;
    int term_cols;
    term_screen_get_size(NULL, &term_cols);
    int width = (term_cols < 84) ? term_cols : 84;
    int left_padding = 3;
    int right_padding = 1;
    int content_width = width - left_padding - right_padding;
    for (int row = 0; row < content_block_size; row++)
    {
        int line_num = start_line + row;
        int screen_row = 4 + row;
        printf("\033[%d;%dH", screen_row, left_padding);
        printf("%-*s", content_width, "");
        printf("\033[%d;%dH", screen_row, left_padding);
        if (lines && line_num < content_size)
            printf("%.*s", content_width, lines[line_num]);
    }
    fflush(stdout);
}

 int handle_common_input(char input_char, int *current_module)
{
    if (input_char == 'Q')
        return -1;
    int new_module = -1;
    if (input_char >= '0' && input_char <= '9') {
        new_module = input_char - '0';
    }
    else if (input_char >= 'a' && input_char <= 'z') {
        new_module = 10 + (input_char - 'a');
    }
    if (new_module >= 0)
        *current_module = new_module;
    return 1;
}

void module_draw(Module modules[], int count, int index)
{
    if (index < 0 || index >= count)
        return;
    Module *m = &modules[index];
    if (m->draw_frame == NULL)
        return;
    printf("\033[?25l");
    fflush(stdout);
    if (m->first_draw) {
        printf("\033[2J\033[H");
        fflush(stdout);
        m->draw_frame();
        m->first_draw = 0;
    }
}

void module_first_draw(int *first_draw, void (*draw_frame)(void))
{
    printf("\033[?25l");
    fflush(stdout);

    if (*first_draw) {
        printf("\033[2J\033[H");
        fflush(stdout);

        draw_frame();
        *first_draw = 0;
    }
}
