#include <stdio.h>
#include <curses.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"

#define MODULE_COUNT 10

ModuleType current_mode = MODULE_NCURSES;

static ModuleType modules[] = {
    MODULE_NCURSES,
    MODULE_NCURSES,
    MODULE_NCURSES,
    MODULE_NCURSES,
    MODULE_NCURSES,
    MODULE_NCURSES,
    MODULE_ANSI,
    MODULE_ANSI,
    MODULE_ANSI,
    MODULE_ANSI
};

int current_content_size = 0;

int module_00_run(void);
int module_01_run(void);
int module_02_run(void);
int module_03_run(void);
int module_04_run(void);
int module_05_run(void);
int module_06_run(void);
int module_07_run(void);
int module_08_run(void);
int module_09_run(void);

int (*module_runs[10])(void) =
{
    module_00_run,
    module_01_run,
    module_02_run,
    module_03_run,
    module_04_run,
    module_05_run,
    module_06_run,
    module_07_run,
    module_08_run,
    module_09_run
};

typedef struct {
    int module_id;
    int current_line;
    int block_size;
    int total;
} ScrollableModule;

static ScrollableModule scrollable_modules[] = {
    {1, 0, 0, 0  },
    {6, 0, 0, 59 },
    {7, 0, 0, 256},
    {8, 0, 0, 256},
    {9, 0, 0, 39 },
};

void common_scroll(int direction, int *current_line, int block_size, int total)
{
    if (block_size >= total) return;
    int new_line = *current_line + direction;
    if (new_line < 0) new_line = 0;
    if (new_line > total - block_size)
        new_line = total - block_size;
    *current_line = new_line;
}

void universal_scroll(int module_id, int direction)
{
    for (size_t i = 0; i < sizeof(scrollable_modules)/sizeof(scrollable_modules[0]); i++)
    {
        if (scrollable_modules[i].module_id == module_id)
        {
            common_scroll(
                direction,
                &scrollable_modules[i].current_line,
                scrollable_modules[i].block_size,
                scrollable_modules[i].total
            );
            break;
        }
    }
}

int universal_get_current_line(int module_id)
{
    for (size_t i = 0; i < sizeof(scrollable_modules)/sizeof(scrollable_modules[0]); i++)
        if (scrollable_modules[i].module_id == module_id)
            return scrollable_modules[i].current_line;

    return 0;
}

void universal_set_current_line(int module_id, int line)
{
    for (size_t i = 0; i < sizeof(scrollable_modules)/sizeof(scrollable_modules[0]); i++)
    {
        if (scrollable_modules[i].module_id == module_id)
        {
            if (line < 0) line = 0;
            if (line > scrollable_modules[i].total - scrollable_modules[i].block_size)
                line = scrollable_modules[i].total - scrollable_modules[i].block_size;
            scrollable_modules[i].current_line = line;
            break;
        }
    }
}

void universal_set_block_size(int module_id, int size)
{
    for (size_t i = 0; i < sizeof(scrollable_modules)/sizeof(scrollable_modules[0]); i++)
        if (scrollable_modules[i].module_id == module_id)
            scrollable_modules[i].block_size = size;
}

typedef int (*module_run_fn)(void);
module_run_fn ncurses_module_runs[] = {
    module_00_run,
    module_01_run,
    module_02_run,
    module_03_run,
    module_04_run,
    module_05_run
};

#define NCURSES_MODULE_COUNT (sizeof(ncurses_module_runs)/sizeof(ncurses_module_runs[0]))

extern int module_06_first_draw;
extern int module_07_first_draw;
extern int module_08_first_draw;
extern int module_09_first_draw;

typedef struct {
    int (*run)(void);
    int *first_draw_flag;
} AnsiModule;

static AnsiModule ansi_modules[] = {
    { module_06_run, &module_06_first_draw },
    { module_07_run, &module_07_first_draw },
    { module_08_run, &module_08_first_draw },
    { module_09_run, &module_09_first_draw },
};

static int run_main_loop(int start_module) {
    int current_module = start_module;
    while (1) {
        ModuleType needed =
            (current_module < MODULE_COUNT)
            ? modules[current_module]
            : MODULE_NCURSES;
        if (needed != current_mode) {
            if (current_mode == MODULE_NCURSES) {
                terminal_control(0);
                term_termios(1, 0, 1, 0, 0, 1, 1, 0);
                printf("\033[2J\033[H");
                fflush(stdout);
            }
            if (needed == MODULE_NCURSES) {
                term_termios(0, 1, 0, 0, 0, 0, 0, 0);
                terminal_control(1);
            }
            if (needed == MODULE_ANSI) {
                if (current_module >= 6 && current_module <= 9) {
                    *ansi_modules[current_module - 6].first_draw_flag = 1;
                }
            }
            current_mode = needed;
        }
        int switched = 0;
        if (current_mode == MODULE_NCURSES) {
            werase(stdscr);
            if (current_module >= 0 && current_module < MODULE_COUNT) {
                if (current_module == 1)
                    universal_set_block_size(1, LINES - 3);
                int ret = module_runs[current_module]();
                if (ret == -1) {
                    switched = 1;
                    current_module = -1;
                } else if (ret != current_module) {
                    switched = 1;
                    current_module = ret;
                }
            } else {
                generic_msg_ncurses(2, 1, 1, 1, -1, "Module: ");
                if (current_module <= 9) {
                    generic_msg_ncurses(VOID, VOID, 0, 7, -1, "%d", current_module);
                } else {
                    generic_msg_ncurses(VOID, VOID, 0, 7, -1, "%c", 'a' + (current_module - 10));
                }
                generic_msg_ncurses(VOID, VOID, 0, 1, -1, " empty");
            }
            wrefresh(stdscr);
        } else {
            if (current_module >= 6 && current_module <= 9) {
                universal_set_block_size(current_module, LINES);
                *ansi_modules[current_module - 6].first_draw_flag = 1;
                module_runs[current_module]();
            } else if (current_module >= 0 && current_module <= 5) {
                printf("\033[1;1HModule %d requires ncurses mode. Press any key...\n", current_module);
            } else {
                printf("\033[1;1HModule: ");
                if (current_module <= 9)
                    printf("%d", current_module);
                else
                    printf("%c", 'a' + (current_module - 10));
                printf(" empty\n");
            }
            fflush(stdout);
        }
        if (switched) {
            if (current_module == -1) {
                break;
            }
            continue;
        }
        if (current_mode == MODULE_NCURSES) {
            int old_module = current_module;
            int result;
            do {
                int ch = getch();
                result = handle_common_input((char)ch, &current_module);
                if (result == -1)
                    break;
            } while (current_module == old_module);

            if (result == -1)
                break;
        } else {
            char ch;
            ssize_t bytes = read(STDIN_FILENO, &ch, 1);
            if (bytes == 1) {
                int result = handle_common_input(ch, &current_module);
                if (result == -1)
                    break;
                if (ch == 27) {
                    char seq[2];
                    bytes = read(STDIN_FILENO, seq, 2);
                    if (bytes == 2 && seq[0] == '[') {
                        if (seq[1] == 'A') {
                            universal_scroll(current_module, -1);
                            if (current_module >= 6 && current_module <= 9) {
                                module_runs[current_module]();
                                continue;
                            }
                        }
                        if (seq[1] == 'B') {
                            universal_scroll(current_module, 1);
                            if (current_module >= 6 && current_module <= 9) {
                                module_runs[current_module]();
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static void terminal_restore(void)
{
    if (current_mode == MODULE_ANSI) {
       term_termios(0, 1, 0, 0, 0, 0, 0, 0);
    } else if (current_mode == MODULE_NCURSES) {
        terminal_control(0);
    }
    write(STDOUT_FILENO, "\033[3J\033[2J", 8);
    write(STDOUT_FILENO, "\033[H", 3);
    printf("\033[?25h");
}

int main(void)
{
    if (terminal_control(1) != 0)
        return 1;
    int current_module = 0;
    run_main_loop(current_module);
    terminal_restore();
    return 0;
}
