#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "analyzer_wrapper.h"

int mod_01_run(const char *filename);
int mod_02_run(const char *filename);
int mod_03_run(const char *filename);
int mod_04_run(const char *filename);
int mod_05_run(const char *filename);
int mod_06_run(const char *filename);
int mod_07_run(const char *filename);
int mod_08_run(const char *filename);
int mod_09_run(const char *filename);
int mod_10_run(const char *filename);
int mod_11_run(const char *filename);

int module_12_run(void);

static void show_file_analysis(WINDOW *parent_win, const char *full_path, const char *filename) {
    int old_cursor = curs_set(1);
        curs_set(0);
    int parent_height, parent_width;
        getmaxyx(parent_win, parent_height, parent_width);
    int parent_start_y, parent_start_x;
        getbegyx(parent_win, parent_start_y, parent_start_x);
    int analysis_height = parent_height - 3;
    int analysis_width = parent_width;
    WINDOW *analysis_win = newwin(analysis_height, analysis_width, parent_start_y + 3, parent_start_x);
    if (!analysis_win) {
        mvwprintw(parent_win, 2, 2, "Failed to create analysis window");
        wgetch(parent_win);
        curs_set(old_cursor);
        return;
    }
    keypad(analysis_win, TRUE);
    generic_frame_ncurses(analysis_win, 0, 0, analysis_width, analysis_height, 0, 2, -1, -1, 1, 0, 1, "FILE ANALYSIS RESULTS");
    AnalysisResult result;
    analysis_result_init(&result);
    if (run_file_analysis(full_path, &result) == 0 && result.line_count > 0) {
        int top_line = 0;
        int ch;
        int analyzing = 1;
        while (analyzing) {
            int rows = getmaxy(analysis_win);
            int cols = getmaxx(analysis_win);
            int visible = rows - 2;
            int max_top = result.line_count - visible;
            if (max_top < 0) max_top = 0;
            if (top_line > max_top) top_line = max_top;
            if (top_line < 0) top_line = 0;
            for (int y = 1; y < rows - 1; y++) {
                for (int x = 1; x < cols - 1; x++) {
                    mvwaddch(analysis_win, y, x, ' ');
                }
            }
            int y = 1;
            for (int i = 0; i < visible && (top_line + i) < result.line_count; i++) {
                const char *current_line = result.lines[top_line + i];
                if (strncmp(current_line, " Module:", 8) == 0) {
                    wattron(analysis_win, COLOR_PAIR(get_color_pair(5, -1)));
                    mvwprintw(analysis_win, y, 1, "%.*s", cols - 3, current_line);
                    wattroff(analysis_win, COLOR_PAIR(get_color_pair(5, -1)));
                } else {
                    mvwprintw(analysis_win, y, 4, "%.*s", cols - 6, current_line);
                }
                y++;
            }
            ch = wgetch(analysis_win);
            switch (ch) {
                case 'Z':
                case 27:
                    analyzing = 0;
                    break;
                case KEY_UP:
                    if (top_line > 0) top_line--;
                    break;
                case KEY_DOWN:
                    if (top_line < max_top) top_line++;
                    break;
                case KEY_PPAGE:
                    top_line -= visible;
                    if (top_line < 0) top_line = 0;
                    break;
                case KEY_NPAGE:
                    top_line += visible;
                    if (top_line > max_top) top_line = max_top;
                    break;
                case KEY_HOME:
                    top_line = 0;
                    break;
                case KEY_END:
                    top_line = max_top;
                    break;
            }
        }
    } else {
        mvwprintw(analysis_win, 2, 2, "Failed to analyze file: %s", filename);
        mvwprintw(analysis_win, 4, 2, "Press any key to continue...");
        wgetch(analysis_win);
    }
    analysis_result_free(&result);
    delwin(analysis_win);
    touchwin(parent_win);
    curs_set(old_cursor);
}

int module_12_run(void) {
    set_color_scheme_flag(' ');
       cycle_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
       module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, 'C', "Complex of modules file analyzer");
    int win_height = 10;
    if (termheight > 14 && termheight < 1000) {
        int new_height = termheight - 4;
        win_height = new_height;
    }
       list_win = newwin(win_height, INNER_WIDTH, 3, 1);
    if (!list_win) {
       list_win = newwin(10, INNER_WIDTH, 3, 1);
    }
    keypad(list_win, TRUE);
    if (getcwd(curr_dir, PATH_MAX) == NULL) {
        strcpy(curr_dir, "/");
    }
    update_file_list();
    if (list_win) {
        top(list_win, curr_dir, &path_visual);
        draw_file_list(list_win);
    }
    int ch;
    int should_exit = 0;
    while (list_win && (ch = wgetch(list_win)) != ERR && !should_exit) {
    int action = handle_module_switch_key(ch);
    if (action == -1 || action > 0 || ch == '0') {
        ungetch(ch);
        return 0;
    }
        if (handle_common_navigation_keys(ch, &selected_index, file_cnt,
                                          curr_dir, &path_visual, &should_exit)) {
            if (should_exit) break;
            draw_file_list(list_win);
            continue;
        }
        if (ch == 10 || ch == KEY_ENTER) {
            if (file_cnt > 0 && selected_index >= 0 && file_list[selected_index].name) {
                if (file_list[selected_index].is_dir) {
                    char new_path[PATH_MAX];
                    if (strlen(curr_dir) + strlen(file_list[selected_index].name) + 2 > PATH_MAX) continue;
                    strcpy(new_path, curr_dir);
                    strcat(new_path, "/");
                    strcat(new_path, file_list[selected_index].name);
                    if (chdir(new_path) == 0) {
                        getcwd(curr_dir, PATH_MAX);
                        update_file_list();
                        selected_index = 0;
                        path_visual = 0;
                        top(list_win, curr_dir, &path_visual);
                        draw_file_list(list_win);
                    }
                } else {
                    char *full_path = xasprintf("%s/%s", curr_dir, file_list[selected_index].name);
                    if (full_path) {
                        show_file_analysis(list_win, full_path, file_list[selected_index].name);
                        free(full_path);
                        top(list_win, curr_dir, &path_visual);
                        draw_file_list(list_win);
                    }
                }
            }
        }
    }
    return 0;
}
