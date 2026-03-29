#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <curses.h>
#include <wchar.h>
#include <wctype.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#include "common.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"

#define MIN_WIDTH 84
#define MIN_HEIGHT 21
#define SAFE_FREE(ptr) do { free(ptr); (ptr) = NULL; } while(0)

int term_width = 0;
int selected_index = 0;
int path_visual = 0;

FileEntry *file_list = NULL;
int file_cnt = 0;

WINDOW *list_win = NULL;

char curr_dir[PATH_MAX] = "";
char **forward_history = NULL;
size_t forward_count = 0;
size_t forward_capacity = 0;

int termheight = 0;
int termwidth = 0;
int content_block_size = 0;
int content_current_line = 0;

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
            mvwprintw(win, start_y + i, start_x, "%-*s",
            width - 2,
            content[*scroll_offset + i]);
        }
    }
    wmove(win, 0, 0);
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

static int file_entry_cmp(const void *a, const void *b) {
    const FileEntry *entry_a = (const FileEntry *)a;
    const FileEntry *entry_b = (const FileEntry *)b;
    if (entry_a->is_dir != entry_b->is_dir) {
        return entry_b->is_dir - entry_a->is_dir;
    }
    return strcasecmp(entry_a->name, entry_b->name);
}

char *xasprintf(const char *fmt, ...)
{
    if (!fmt)
        return NULL;
    va_list ap, ap_copy;
    va_start(ap, fmt);
    va_copy(ap_copy, ap);
    char *res = NULL;
    int len = vasprintf(&res, fmt, ap_copy);
    va_end(ap_copy);
    va_end(ap);
    if (len < 0) {
        free(res);
        res = NULL;
    }
    return res;
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

void handle_path_scroll(int ch, int *offset, const char *dir_path) {
    if (!offset || !dir_path) return;
    int max_width = INNER_WIDTH - 4;
    int visual_width = 0;
    {
        wchar_t dummy[8] = {0};
        truncate_and_measure_wstring(dir_path, max_width, dummy,
                                     sizeof(dummy) / sizeof(wchar_t),
                                     false,
                                     L"..",
                                     0,
                                     &visual_width);
    }
    int max_off = (visual_width > max_width) ? visual_width - max_width : 0;
    if (ch == 'l' || ch == 'L') {
        if (*offset > 0) (*offset)--;
    } else if (ch == 'r' || ch == 'R') {
        if (*offset < max_off) (*offset)++;
    }
}

int handle_common_navigation_keys(int ch, int *p_selected_index, int file_count, char *current_dir, int *path_visual_offset, int *should_exit)
{
    if (!p_selected_index || !current_dir || !path_visual_offset || !should_exit)
        return 0;
    *should_exit = 0;
    if (ch == KEY_UP || ch == KEY_DOWN) {
        if (file_count > 0) {
            if (ch == KEY_UP) {
                if (*p_selected_index > 0) (*p_selected_index)--;
                else *p_selected_index = file_count - 1;
            } else {
                if (*p_selected_index < file_count - 1) (*p_selected_index)++;
                else *p_selected_index = 0;
            }
        }
        return 1;
    }
    if (ch == KEY_LEFT || ch == KEY_RIGHT) {
        if (ch == KEY_LEFT) {
            if (strcmp(current_dir, "/") == 0) return 1;
            char temp_path[PATH_MAX];
            strncpy(temp_path, current_dir, sizeof(temp_path)-1);
            temp_path[sizeof(temp_path)-1] = '\0';
            char *last_slash = strrchr(temp_path, '/');
            if (last_slash == temp_path) strcpy(temp_path, "/");
            else if (last_slash) *last_slash = '\0';
            if (navigate_history(HISTORY_ADD_FORWARD, current_dir) == 0) {
                if (navigate_to(temp_path, false) != 0) {
                    if (forward_count > 0) {
                        SAFE_FREE(forward_history[forward_count - 1]);
                        forward_count--;
                    }
                }
            }
        } else {
            if (forward_count > 0) {
                char *next_dir = forward_history[forward_count - 1];
                    if (navigate_to(next_dir, true) == 0) {
                    SAFE_FREE(forward_history[forward_count - 1]);
                    forward_count--;
                }
            }
        }
        return 1;
    }
    if (ch == 'l' || ch == 'r' || ch == 'L' || ch == 'R') {
        handle_path_scroll(ch, path_visual_offset, current_dir);
        return 1;
    }
    return 0;
}

int navigate_history(HistoryAction action, const char *path) {
    switch (action) {
        case HISTORY_ADD_FORWARD:
            if (!path) return -1;
            if (forward_count >= forward_capacity) {
                size_t new_capacity = forward_capacity == 0 ? 10UL : forward_capacity * 2;
                char **temp = realloc(forward_history, new_capacity * sizeof(char *));
                if (!temp) return -1;
                forward_history = temp;
                memset(forward_history + forward_count, 0,
                       (new_capacity - forward_count) * sizeof(char *));
                forward_capacity = new_capacity;
            }
            forward_history[forward_count] = strdup(path);
            if (!forward_history[forward_count]) return -1;
            forward_count++;
            return 0;
        case HISTORY_GET_LAST:
            return (forward_count > 0) ? 0 : -1;
        case HISTORY_CLEAR:
            for (size_t i = 0; i < forward_count; i++) {
                free(forward_history[i]);
                forward_history[i] = NULL;
            }
            forward_count = 0;
            return 0;
        default:
            return -1;
    }
}

void update_file_list(void) {
    free_entries((void**)&file_list, &file_cnt, OUTPUT_MODE_FILE_LIST);
    int count = 0;
    FileEntry *entries = NULL;
    if (scan_directory_ex(curr_dir, 0, OUTPUT_MODE_FILE_LIST,
                     (void **)&entries, &count) != 0) {
        file_list = calloc(1, sizeof(FileEntry));
        file_list[0].name = strdup("(access denied)");
        file_list[0].is_dir = 0;
        file_cnt = 1;
        selected_index = 0;
        return;
    }
    if (count == 0) {
        file_list = calloc(1, sizeof(FileEntry));
        if (file_list) {
            file_list[0].name = strdup("Directory is empty or not enough memory.");
            file_list[0].is_dir = 0;
            file_cnt = 1;
        }
            selected_index = 0;
        free(entries);
        return;
    }
    qsort(entries, (size_t)count, sizeof(FileEntry), file_entry_cmp);
    file_list = entries;
    file_cnt = count;
    selected_index = 0;
}

int scan_directory_ex(const char *dir_path, unsigned int category_filter, OutputMode output_mode, void **entries_out, int *count_out) {
                      DIR *dir = opendir(dir_path);
if (!dir) {
    return -1;
}
    size_t capacity = 100;
    size_t entry_size = (output_mode == OUTPUT_MODE_PLAYLIST) ?
                        sizeof(char *) : sizeof(FileEntry);
    void *entries = calloc(capacity, entry_size);
    if (!entries) {
        closedir(dir);
        return -1;
    }
    struct dirent *entry;
    size_t idx = 0;
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') continue;
if (category_filter != 0) {
    const char *dname = entry->d_name;
    if (!dname) continue;
    size_t len = 0;
    for (len = 0; len < 256; len++) {
        if (dname[len] == '\0') break;
        if (dname[len] < 32 || dname[len] > 126) {
            break;
        }
    }
    if (len == 0 || len >= 256 || dname[len] != '\0') {
        continue;
    }
    if (!is_file_category(dname, category_filter)) {
        continue;
    }
}
        if (idx >= capacity) {
            capacity *= 2;
            void *new_entries = realloc(entries, capacity * entry_size);
            if (!new_entries) {
                if (output_mode == OUTPUT_MODE_PLAYLIST) {
                 free_entries(&entries, (int*)&idx, output_mode);
                }
                closedir(dir);
                return -1;
            }
            entries = new_entries;
        }
        if (output_mode == OUTPUT_MODE_PLAYLIST) {
            char *full_path = xasprintf("%s/%s", dir_path, entry->d_name);
            char resolved_path[PATH_MAX];
            if (realpath(full_path, resolved_path) != NULL) {
                ((char **)entries)[idx] = strdup(resolved_path);
            } else {
                ((char **)entries)[idx] = full_path;
            }
            if (!((char **)entries)[idx]) {
                free(full_path);
                continue;
            }
        } else {
            char *name = strdup(entry->d_name);
            if (!name) continue;
            char *full_path = xasprintf("%s/%s", dir_path, entry->d_name);
            struct stat st;
            if (lstat(full_path, &st) == -1) {
                ((FileEntry *)entries)[idx].is_dir = 0;
            } else {
                ((FileEntry *)entries)[idx].is_dir = S_ISDIR(st.st_mode) ? 1 : 0;
            }
            ((FileEntry *)entries)[idx].name = name;
            free(full_path);
        }
        idx++;
    }
    closedir(dir);
    if (idx == 0) {
        free(entries);
        *entries_out = NULL;
        *count_out = 0;
        return 0;
    }
    qsort(entries, idx, sizeof(FileEntry), file_entry_cmp);
    *entries_out = entries;
    *count_out = (int)idx;
    return 0;
}

void top(WINDOW *win, const char *dir_path, int *visual_offset) {
    cycle_frame_ncurses(win, 0, 1, 82, 3, 0, 2, -1, -1, 1, 0, 1, "PATH");
    char path_display[PATH_MAX];
    strncpy(path_display, dir_path, sizeof(path_display) - 1);
    path_display[sizeof(path_display) - 1] = '\0';
    wchar_t wpath[512];
    int max_path_width = INNER_WIDTH - 4;
    int total_width = 0;
    {
        wchar_t dummy[8] = {0};
        int measured = 0;
        truncate_and_measure_wstring(path_display,
                                     max_path_width,
                                     dummy,
                                     sizeof(dummy) / sizeof(wchar_t),
                                     false,
                                     L"..",
                                     0,
                                     &measured);
        total_width = measured;
    }
    int max_offset = 0;
    if (total_width > max_path_width) {
        max_offset = total_width - max_path_width;
    }
    if (*visual_offset > max_offset) *visual_offset = max_offset;
    int result = prepare_display_wstring(path_display, max_path_width, wpath,
                          sizeof(wpath)/sizeof(wchar_t), 0, L"..",
                          *visual_offset, 0);
    if (result == 0) {
        size_t wlen = wcslen(wpath);
        if (wlen > 0) {
            wattron(win, COLOR_PAIR(get_color_pair(7, -1)));
            mvwaddnwstr(win, 1, 2, wpath, (int)wlen);
            wattroff(win, COLOR_PAIR(get_color_pair(7, -1)));
        }
    }
}

void draw_file_list(WINDOW *win) {
    int max_y, max_x, visible_lines, start_index, end_index;
    int result = prepare_file_list_display(win, &max_y, &max_x, &visible_lines, &start_index, &end_index);
    if (result <= 1) return;
{
        int cursor_end = 2 + CURSOR_WIDTH;
        for (int i = start_index; i < end_index; i++) {
            if (!file_list[i].name) continue;
            int row = i - start_index + 4;
wchar_t wname[CURSOR_WIDTH + 4] = {0};
int reserve =
    (file_list[i].is_dir ? 1 : 0) +
    2;
prepare_display_wstring(
    file_list[i].name,
    CURSOR_WIDTH - reserve,
    wname,
    sizeof(wname) / sizeof(wchar_t),
    file_list[i].is_dir,
    L"..",
    0,
    1
);
  int printed = wcswidth(wname, wcslen(wname));
if (!file_list[i].is_dir) {
    size_t wl = wcslen(wname);
    if (wl >= 2 &&
        wname[wl - 1] == L'.' &&
        wname[wl - 2] == L'.')
    {
        int last = wcwidth(wname[wl - 3]);
        if (last == 2) {
            printed += 1;
        }
    }
}
    if (i == selected_index) {
       wattron(win, COLOR_PAIR(get_color_pair(2, -1)));
        wchar_t fill_ch1 = L'▒';
    for (int x = 2; x < cursor_end; x++) {
        mvwaddnwstr(win, row, x, &fill_ch1, 1);
}
       wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
int text_color = get_color_pair(2, -1);
    wattron(win, COLOR_PAIR(text_color));
    mvwaddwstr(win, row, 3, wname);
    wattroff(win, COLOR_PAIR(text_color));
    wattron(win, COLOR_PAIR(get_color_pair(2, -1)));
    wchar_t fill_ch2 = L'▒';
    for (int x = 3 + printed; x < cursor_end; x++) {
    mvwaddnwstr(win, row, x, &fill_ch2, 1);
}
 wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
    } else {
      int text_color = get_text_module_file_color(file_list[i].name, file_list[i].is_dir, current_color_flag);
        wattron(win, COLOR_PAIR(text_color));
        mvwaddwstr(win, row, 3, wname);
        wattroff(win, COLOR_PAIR(text_color));
        wchar_t space_ch = L' ';
    for (int x = 3 + printed; x < cursor_end; x++) {
    mvwaddnwstr(win, row, x, &space_ch, 1);
}
    }
}
    if (file_cnt > visible_lines) {
        int scroll_height = max_y - 5;
        int bar_x = 2 + CURSOR_WIDTH + 1;
        draw_scrollbar(win, 4, scroll_height, file_cnt, visible_lines, start_index, bar_x);
    }
    int last_list_row = end_index - start_index + 4;
    int safe_clear_end = max_y - 5;
    if (safe_clear_end < 0) safe_clear_end = 0;
    if (last_list_row < safe_clear_end) {
        clear_rect(win, last_list_row, safe_clear_end, 1, max_x - 1);
    }
}
}

int prepare_file_list_display(WINDOW *win, int *out_max_y, int *out_max_x, int *out_visible_lines, int *out_start_index, int *out_end_index) {
    if (!win) return -1;
    top(win, curr_dir, &path_visual);
    int max_y = getmaxy(win);
    int max_x = getmaxx(win);
    int max_y_local = getmaxy(win);
    int usable_height = max_y_local - 3;
    if (usable_height < 3) usable_height = 3;
       cycle_frame_ncurses(win, 0, 4, INNER_WIDTH, usable_height, 0, 2, -1, -1, 1, 0, 1, "FILES & DIRECTORIES");
       wnoutrefresh(win);
    if (out_max_y) *out_max_y = max_y;
    if (out_max_x) *out_max_x = max_x;
    if (file_cnt == 0 || !file_list) {
        mvwprintw(win, 3, 1, "(empty)");
        return 0;
    }
    int visible_lines = (max_y < 12) ? 1 : max_y - 5;
    int start_index = 0;
    int end_index = file_cnt;
    if (file_cnt > 0 && visible_lines > 0) {
        if (file_cnt > visible_lines) {
            start_index = selected_index - (visible_lines / 2);
            if (start_index < 0) start_index = 0;
            if (start_index > file_cnt - visible_lines) {
                start_index = file_cnt - visible_lines;
            }
        }
        end_index = start_index + visible_lines;
        if (end_index > file_cnt) end_index = file_cnt;
    }
    if (out_visible_lines) *out_visible_lines = visible_lines;
    if (out_start_index) *out_start_index = start_index;
    if (out_end_index) *out_end_index = end_index;
    if (file_cnt == 1 && file_list[0].name &&
        strcmp(file_list[0].name, "Directory is empty or not enough memory.") == 0) {
        int msg_row = 4 + (visible_lines / 2);
        if (msg_row < 5) msg_row = 5;
        if (msg_row >= max_y - 5) msg_row = max_y - 6;
           wattron(win, COLOR_PAIR(get_color_pair(124, -1)));
        const char* msg = file_list[0].name;
        wchar_t wmsg[CURSOR_WIDTH + 4] = {0};
        prepare_display_wstring(msg, CURSOR_WIDTH, wmsg,
                              sizeof(wmsg)/sizeof(wchar_t), 0, L"..", 0, 0);
        mvwaddwstr(win, msg_row, 3, wmsg);
        wattroff(win, COLOR_PAIR(get_color_pair(124, -1)));
        return 1;
    }
    return 2;
}

 void draw_scrollbar(WINDOW *win, int start_y, int height, int total_files, int visible_lines, int start_index, int bar_x) {
    if (!win) return;
    if (total_files <= visible_lines) return;
       float ratio = (float)height / (float)total_files;
       int scroll_bar_height = (int)((float)visible_lines * ratio + 0.5f);
    if (scroll_bar_height < 1) scroll_bar_height = 1;
    if (scroll_bar_height > height) scroll_bar_height = height;
       int max_scroll_offset = total_files - visible_lines;
       int scroll_pos = start_y;
    if (max_scroll_offset > 0) {
        float normalized_pos = (float)start_index / (float)max_scroll_offset;
        scroll_pos = start_y + (int)(normalized_pos * (float)(height - scroll_bar_height));
        if (scroll_pos < start_y) scroll_pos = start_y;
        int max_pos = start_y + height - scroll_bar_height;
        if (max_pos < start_y) max_pos = start_y;
        if (scroll_pos > max_pos)
            scroll_pos = max_pos;
    }
    wchar_t empty_char = SCROLL_EMPTY;
    wchar_t fill_char = SCROLL_FILLED;
    wattron(win, COLOR_PAIR(get_color_pair(2, -1)));
    draw_fill_line(win, start_y, bar_x, height, &empty_char, 0, 1);
    draw_fill_line(win, scroll_pos, bar_x, scroll_bar_height, &fill_char, 0, 1);
    wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
}

void clear_rect(WINDOW *win, int start_y, int end_y, int start_x, int end_x) {
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            mvwaddch(win, y, x, ' ');
        }
    }
}

void draw_fill_line(WINDOW *win, int start_y, int start_x, int length, const void *symbol, int is_horizontal, int is_wide) {
    int y = start_y;
    int x = start_x;
    for (int i = 0; i < length; i++) {
        if (is_wide) {
            mvwaddnwstr(win, y, x, (const wchar_t *)symbol, 1);
        } else {
            mvwprintw(win, y, x, "%s", (const char *)symbol);
        }
        if (is_horizontal) x++; else y++;
    }
}

char current_color_flag = ' ';
void set_color_scheme_flag(char flag) {
    current_color_flag = flag;
}

int handle_module_switch_key(int ch)
{
    if (ch == 'Q' || ch == 'q') {
        return -1;
    }
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return 10 + (ch - 'a');
    }
    return 0;
}

void free_entries(void **entries_ptr, int *count_ptr, OutputMode mode)
{
    if (!entries_ptr || !*entries_ptr) {
        if (count_ptr) *count_ptr = 0;
        return;
    }
    void *entries = *entries_ptr;
    if (mode == OUTPUT_MODE_FILE_LIST) {
        FileEntry *list = (FileEntry *)entries;
        for (int i = 0; i < *count_ptr; i++) {
            SAFE_FREE(list[i].name);
        }
    }
    else if (mode == OUTPUT_MODE_PLAYLIST) {
        char **arr = (char **)entries;
        for (int i = 0; i < *count_ptr; i++) {
            SAFE_FREE(arr[i]);
        }
    }
    SAFE_FREE(entries);
    *entries_ptr = NULL;
    if (count_ptr) {
        *count_ptr = 0;
    }
}

int navigate_to(const char *target_dir, bool refresh_screen)
{
    if (!target_dir) {
        return -1;
    }
    if (chdir(target_dir) != 0) {
        return -1;
    }
    if (getcwd(curr_dir, PATH_MAX) == NULL) {
        return -1;
    }
    update_file_list();
    selected_index = 0;
    path_visual = 0;
    if (refresh_screen && list_win != NULL) {
        draw_file_list(list_win);
        top(list_win, curr_dir, &path_visual);
        wnoutrefresh(list_win);
    }
    return 0;
}

int truncate_and_measure_wstring(
    const char *src,
    int max_visual_width,
    wchar_t *dest,
    size_t dest_size,
    bool is_directory,
    const wchar_t *ellipsis,
    int scroll_offset,
    int *out_visual_width)
{
    if (!src || !dest || dest_size == 0) {
        if (out_visual_width) *out_visual_width = 0;
        return -1;
    }
    if (max_visual_width <= 0) {
        dest[0] = L'\0';
        if (out_visual_width) *out_visual_width = 0;
        return 0;
    }
    size_t src_len = strlen(src);
    wchar_t *wstr = calloc(src_len + 1, sizeof(wchar_t));
    if (!wstr) {
        if (out_visual_width) *out_visual_width = 0;
        return -1;
    }
    mbstate_t state = {0};
    const char *ptr = src;
    size_t wlen = mbsrtowcs(wstr, &ptr, src_len + 1, &state);
    if (wlen == (size_t)-1) {
        for (size_t i = 0; i < src_len; i++) {
            wstr[i] = (wchar_t)(unsigned char)src[i];
        }
        wstr[src_len] = L'\0';
        wlen = src_len;
    }
    int total_visual = 0;
    for (size_t i = 0; i < wlen; i++) {
        int cw = wcwidth(wstr[i]);
        if (cw > 0) total_visual += cw;
    }
    if (is_directory && total_visual + 1 <= max_visual_width) {
        if (wlen + 1 < dest_size) {
            wstr[wlen] = L'/';
            wstr[wlen + 1] = L'\0';
            total_visual += 1;
            wlen++;
        }
    }
    int ellipsis_len = ellipsis ? (int)wcslen(ellipsis) : 0;
    int avail_width = max_visual_width;
    if (scroll_offset > 0) {
        int skipped = 0;
        size_t start = 0;
        while (start < wlen && skipped < scroll_offset) {
            int cw = wcwidth(wstr[start]);
            if (cw > 0) skipped += cw;
            start++;
        }
        memmove(wstr, wstr + start, (wlen - start + 1) * sizeof(wchar_t));
        wlen -= start;
        total_visual = 0;
        for (size_t i = 0; i < wlen; i++) {
            int cw = wcwidth(wstr[i]);
            if (cw > 0) total_visual += cw;
        }
    }
    if (total_visual > avail_width && ellipsis_len > 0) {
        int cut_width = avail_width - ellipsis_len;
        if (cut_width < 0) cut_width = 0;
        size_t i = 0;
        int current_width = 0;
        while (i < wlen && current_width < cut_width) {
            int cw = wcwidth(wstr[i]);
            if (cw > 0) current_width += cw;
            i++;
        }
        wlen = i;
        if (wlen + (size_t)ellipsis_len < dest_size) {
            wmemcpy(dest, wstr, wlen);
            wmemcpy(dest + wlen, ellipsis, (size_t)ellipsis_len);
            dest[wlen + (size_t)ellipsis_len] = L'\0';
            if (out_visual_width) *out_visual_width = avail_width;
        } else {
            dest[0] = L'\0';
            if (out_visual_width) *out_visual_width = 0;
        }
    } else {
        if (wlen < dest_size) {
            wmemcpy(dest, wstr, wlen + 1);
            if (out_visual_width) *out_visual_width = total_visual;
        } else {
            dest[0] = L'\0';
            if (out_visual_width) *out_visual_width = 0;
        }
    }
    free(wstr);
    return 0;
}
