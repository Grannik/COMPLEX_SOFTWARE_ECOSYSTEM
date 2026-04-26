#define _GNU_SOURCE
#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <time.h>
#include <termios.h>
#include <locale.h>
#include <curses.h>
#include <ncursesw/ncurses.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#include "common.h"
#include "libstrprepare/libstrprepare.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"

PlayerControl player_control = {
    .filename = NULL,
    .pause = 0,
    .stop = 0,
    .quit = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
    .current_file = NULL,
    .current_filename = NULL,
    .paused = 0,
    .loop_mode = 0,
    .playlist = NULL,
    .playlist_size = 0,
    .playlist_capacity = 0,
    .current_track = 0,
    .playlist_mode = 0,
    .seek_delta = 0,
    .duration = 0.0,
    .bytes_read = 0LL,
    .is_silent = 0,
    .fading_out = 0,
    .fading_in = 0,
    .current_fade = FADE_STEPS,
    .playlist_dir = NULL,
};

extern PlayerControl player_control;
bool user_moved_cursor = false;
int path_display_width = 78;
int selected_index = 0;
int path_visual = 0;
FileEntry *file_list = NULL;
int file_cnt = 0;
WINDOW *list_win = NULL;
char curr_dir[PATH_MAX] = "";
char **forward_history = NULL;
size_t forward_count = 0;
size_t forward_capacity = 0;
int content_block_size = 0;
int content_current_line = 0;

int playlist_cmp(const void *a, const void *b);
void handle_pathscroll(int ch, int *offset, const char *dir_path, int max_path_width);
void free_names(void *entries, int count, int is_file_entry);

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

int handle_module_switch_key(int ch)
{
    if (ch == 'Q') {
        return -1;
    }
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return 10 + (ch - 'a');
    }
    return -2;
}
//-=-
 int file_entry_cmp(const void *a, const void *b) {
    const FileEntry *entry_a = (const FileEntry *)a;
    const FileEntry *entry_b = (const FileEntry *)b;
    if (entry_a->is_dir != entry_b->is_dir) {
        return entry_b->is_dir - entry_a->is_dir;
    }
    return strcasecmp(entry_a->name, entry_b->name);
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

void handle_pathscroll(int ch, int *offset, const char *dir_path, int max_path_width)
{
    if (!offset || !dir_path) return;
    char display_path[PATH_MAX] = {0};
if (selected_index >= 0 &&
    selected_index < file_cnt &&
    file_list &&
    file_list[selected_index].name)
{
    size_t len = strlen(dir_path);
    const char *sep = (len > 0 && dir_path[len - 1] == '/') ? "" : "/";
    snprintf(display_path, sizeof(display_path),
             "%s%s%s",
             dir_path,
             sep,
             file_list[selected_index].name);
}
else
{
    strncpy(display_path, dir_path, sizeof(display_path) - 1);
    display_path[sizeof(display_path) - 1] = '\0';
}
    int visual_width = calculate_visual_width(display_path);
    int max_off = (visual_width > max_path_width) ? visual_width - max_path_width : 0;
    if (ch == '[') {
        *offset += 5;
        if (*offset > max_off) *offset = max_off;
    }
    else if (ch == ']') {
        *offset -= 5;
        if (*offset < 0) *offset = 0;
    }
    path_form(list_win, 1, 1, dir_path, offset, max_path_width, 0);
    wnoutrefresh(list_win);
}

int handle_common_navigationkeys(int ch, int *p_selected_index, int file_count, char *current_dir, int *path_visual_offset, int *should_exit, int path_vr)
{
    if (!p_selected_index || !current_dir || !path_visual_offset || !should_exit)
        return 0;
    *should_exit = 0;
    if (ch == KEY_UP || ch == KEY_DOWN) {
        user_moved_cursor = true;
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
        user_moved_cursor = true;
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
        if (list_win != NULL) {
            werase(list_win);
            update_file_list();
        }
        return 1;
    }
if (ch == '[' || ch == ']') {
    user_moved_cursor = true;
      handle_pathscroll(ch, path_visual_offset, current_dir, path_vr);
    if (list_win != NULL) {
        wnoutrefresh(list_win);
    }
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

 void free_names(void *entries, int count, int is_file_entry)
{
    if (!entries) return;

    if (is_file_entry) {
        FileEntry *list = (FileEntry *)entries;
        for (int i = 0; i < count; i++) {
            SAFE_FREE(list[i].name);
        }
    } else {
        char **arr = (char **)entries;
        for (int i = 0; i < count; i++) {
            SAFE_FREE(arr[i]);
        }
    }
    free(entries);
}

 int playlist_cmp(const void *a, const void *b)
{
    const char *path_a = *(const char * const *)a;
    const char *name_a = strrchr(path_a, '/') ? strrchr(path_a, '/') + 1 : path_a;
    const char *path_b = *(const char * const *)b;
    const char *name_b = strrchr(path_b, '/') ? strrchr(path_b, '/') + 1 : path_b;
    wchar_t *w_a = NULL, *w_b = NULL;
    size_t len_a = 0, len_b = 0;
    convert_to_wchar(name_a, &w_a, &len_a);
    convert_to_wchar(name_b, &w_b, &len_b);
    int res = wcscoll(w_a, w_b);
    free(w_a);
    free(w_b);
    return res;
}

int scan_directory_ex(const char *dir_path, unsigned int category_filter, OutputMode output_mode, void **entries_out, int *count_out)
{
    if (!dir_path || !entries_out || !count_out) {
        return -1;
    }
    DIR *dir = opendir(dir_path);
    if (!dir) {
        frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION,
                      "Access denied to: %s", dir_path);
        return -1;
    }
    size_t capacity = 128;
    size_t entry_size = (output_mode == OUTPUT_MODE_PLAYLIST) ?
                        sizeof(char *) : sizeof(FileEntry);
    void *entries = calloc(capacity, entry_size);
    if (!entries) {
        closedir(dir);
        return -1;
    }
    struct dirent *entry;
    size_t idx = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        if (category_filter != 0 &&
            !is_file_category(entry->d_name, category_filter)) {
            continue;
        }
        if (idx >= capacity) {
            capacity *= 2;
            void *new_entries = realloc(entries, capacity * entry_size);
            if (!new_entries) {
                free_names(entries, (int)idx,
                           (output_mode == OUTPUT_MODE_PLAYLIST) ? 0 : 1);
                closedir(dir);
                return -1;
            }
            entries = new_entries;
        }
        if (output_mode == OUTPUT_MODE_PLAYLIST) {
            char *full_path = xasprintf("%s/%s", dir_path, entry->d_name);
            if (!full_path) {
                continue;
            }
            char resolved[PATH_MAX];
            if (realpath(full_path, resolved) != NULL) {
                ((char **)entries)[idx] = strdup(resolved);
                free(full_path);
            } else {
                ((char **)entries)[idx] = full_path;
            }
            if (!((char **)entries)[idx]) {
                free(full_path);
                continue;
            }
        }
        else {
            char *name = strdup(entry->d_name);
            if (!name) continue;
            char *full_path = xasprintf("%s/%s", dir_path, entry->d_name);
            int is_dir_flag = 0;
            if (full_path) {
                struct stat st;
                if (lstat(full_path, &st) == 0) {
                    is_dir_flag = S_ISDIR(st.st_mode) ? 1 : 0;
                }
                free(full_path);
            }
            FileEntry *fe = (FileEntry *)entries;
            fe[idx].name = name;
            fe[idx].is_dir = is_dir_flag;
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
    if (output_mode == OUTPUT_MODE_PLAYLIST) {
        setlocale(LC_COLLATE, "");
        qsort(entries, idx, sizeof(char *), playlist_cmp);
    } else {
        qsort(entries, idx, sizeof(FileEntry), file_entry_cmp);
    }
    *entries_out = entries;
    *count_out = (int)idx;
    return 0;
}

void path_form(WINDOW *win, int x, int y, const char *dir_path, int *visual_offset, int max_path_width, int mode)
{
    char final_display[PATH_MAX] = {0};
    if (selected_index >= 0 && selected_index < file_cnt && file_list && file_list[selected_index].name) {
        size_t len = strlen(dir_path);
        const char *sep = (len > 0 && dir_path[len - 1] == '/') ? "" : "/";
        snprintf(final_display, sizeof(final_display), "%s%s%s", dir_path, sep, file_list[selected_index].name);
    } else {
        strncpy(final_display, dir_path, sizeof(final_display) - 1);
    }
    int total_width = calculate_visual_width(final_display);
    int max_offset = (total_width > max_path_width) ? (total_width - max_path_width) : 0;
    if (*visual_offset > max_offset) *visual_offset = max_offset;
    if (*visual_offset < 0) *visual_offset = 0;
    wchar_t wpath[PATH_MAX];
    int result = prepare_display_wstring(final_display, max_path_width, wpath, sizeof(wpath)/sizeof(wchar_t), 0, L"...", *visual_offset, 2);
    if (result != 0) return;
    size_t wlen = wcslen(wpath);
    if (mode == 0) {
        wmove(win, y, x);
        wclrtoeol(win);
        if (wlen > 0) {
            wattron(win, COLOR_PAIR(get_color_pair(5, -1)));
            mvwaddstr(win, y, x, "PATH:");
            wattroff(win, COLOR_PAIR(get_color_pair(5, -1)));
            wattron(win, COLOR_PAIR(get_color_pair(7, -1)));
            mvwaddnwstr(win, y, x + 6, wpath, (int)wlen);
            wattroff(win, COLOR_PAIR(get_color_pair(7, -1)));
        }
    } else if (mode == 1) {
        cycle_frame_ncurses(win, 0, 1, max_path_width + 4, 3, 0, 2, -1, -1, 1, 0, 1, "PATH");
        if (wlen > 0) {
            wattron(win, COLOR_PAIR(get_color_pair(7, -1)));
            mvwaddnwstr(win, y, x + 1, wpath, (int)wlen);
            wattroff(win, COLOR_PAIR(get_color_pair(7, -1)));
        }
    }
}

void draw_file_list(WINDOW *win) {
    int max_y, max_x, visible_lines, start_index, end_index;
    int result = prepare_file_list_display(win, &max_y, &max_x, &visible_lines, &start_index, &end_index, 3);
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
prepare_display_wstring(file_list[i].name, CURSOR_WIDTH - reserve, wname, sizeof(wname) / sizeof(wchar_t), file_list[i].is_dir, L"..", 0, 2);
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

int prepare_file_list_display(WINDOW *win, int *out_max_y, int *out_max_x, int *out_visible_lines, int *out_start_index, int *out_end_index, int minus) {
    if (!win) return -1;
        path_form(list_win, 1, 1, curr_dir, &path_visual, path_display_width, 1);
    int max_y = getmaxy(win);
    int max_x = getmaxx(win);
    int max_y_local = getmaxy(win);
    int usable_height = max_y_local - minus;
    if (usable_height < 3) usable_height = 3;
       cycle_frame_ncurses(win, 0, 4, INNER_WIDTH, usable_height, 0, 2, -1, -1, 1, 0, 1, "FILES & DIRECTORIES");
       wnoutrefresh(win);
    if (out_max_y) *out_max_y = max_y;
    if (out_max_x) *out_max_x = max_x;
    if (file_cnt == 0 || !file_list) {
        mvwprintw(win, 3, 1, "(empty)");
        return 0;
    }
    int visible_lines = (max_y < 12) ? 1 : usable_height - 2;
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
        prepare_display_wstring(msg, CURSOR_WIDTH, wmsg, sizeof(wmsg)/sizeof(wchar_t), 0, L"..", 0, 2);
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
/*
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
*/
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
        path_form(list_win, 1, 1, curr_dir, &path_visual, path_display_width, 1);
        wnoutrefresh(list_win);
    }
    return 0;
}

char root_dir[PATH_MAX] = "";
int colors_initialized = 0;
char status_msg[256] = "";
int show_status = 0;
time_t status_start_time = 0;

const char *msg_get_title_text(TitleType type) {
    static const char *title_texts[] = {
        "PLAYBACK TIME & PROGRESS",
        "NOTIFICATION",
        "VIEW INFO",
        "WARNING",
        "DEBUG",
        "INFO",
        "TEXT",
        ""
    };
    if (type >= 0 && type <= TITLE_EMPTY) {
        return title_texts[type];
    }
    return "";
}

static struct {
    char text[256];
    MessageType msg_type;
    TitleType title_type;
    time_t show_time;
    int visible;
} current_message = {0};

void msg_system_init(void) {
    memset(&current_message, 0, sizeof(current_message));
    extern char *__progname;
    if (__progname != NULL) {
        if (strstr(__progname, "text_module") != NULL) {
            current_message.title_type = TITLE_VIEW_INFO;
        }
        else if (strstr(__progname, "terminalNavigatorRaw") != NULL) {
            current_message.title_type = TITLE_PLAYBACK_TIME_PROGRESS;
        }
        else {
            current_message.title_type = TITLE_VIEW_INFO;
        }
    } else {
        current_message.title_type = TITLE_VIEW_INFO;
    }
}

void frame_control(TitleType title_type, MessageType msg_type,
                   const char *fmt, ...) {
    if (fmt != TEXT_KEEP) {
        if (fmt && fmt[0] != '\0') {
            va_list ap;
            va_start(ap, fmt);
            vsnprintf(current_message.text, sizeof(current_message.text), fmt, ap);
            va_end(ap);
            current_message.visible = 1;
        } else {
            current_message.text[0] = '\0';
            current_message.visible = 0;
        }
    }
    if (title_type != TITLE_KEEP) {
        current_message.title_type = title_type;
    }
    if (msg_type != COLOR_KEEP) {
        current_message.msg_type = msg_type;
        if (msg_type == MSG_NOTIFICATION) {
            current_message.show_time = time(NULL);
        }
    }
}

void msg_update_timeouts(void) {
    if (current_message.visible &&
        current_message.msg_type == MSG_NOTIFICATION) {
        if (time(NULL) - current_message.show_time >= 5) {
            frame_control(TITLE_VIEW_INFO, MSG_NORMAL, " First line: | Last line: | Total lines: | <->:");
        }
    }
}

void msg_clear(void) {
    current_message.visible = 0;
    current_message.text[0] = '\0';
    current_message.title_type = TITLE_VIEW_INFO;
    current_message.msg_type = 0;
}

const char *msg_get_current_text(void) {
    return current_message.visible ? current_message.text : NULL;
}

MessageType msg_get_current_msg_type(void) {
    return current_message.msg_type;
}

TitleType msg_get_current_title_type(void) {
    return current_message.title_type;
}

int msg_is_visible(void) {
    return current_message.visible;
}

const char *get_last_forward(void) {
    if (forward_count > 0) {
        return forward_history[forward_count - 1];
    }
    return NULL;
}

int change_directory(const char *dir_name, const char *base_path, char *current_dir_buf, size_t buf_size) {
    if (!dir_name || !base_path || !current_dir_buf) {
        return -1;
    }
    char full_path[PATH_MAX];
    if (snprintf(full_path, sizeof(full_path), "%s/%s", base_path, dir_name) >= (int)sizeof(full_path)) {
        frame_control(TITLE_WARNING, MSG_WARNING, "Path too long: %s/%s", base_path, dir_name);
        return -1;
    }
    int dir_fd = openat(AT_FDCWD, dir_name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (dir_fd == -1) {
        frame_control(TITLE_WARNING, MSG_WARNING, ACCESS_DENIED_MSG, full_path);
        return -1;
    }
    struct stat st;
    if (fstat(dir_fd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        frame_control(TITLE_WARNING, MSG_WARNING, "Not a directory or state changed: %s", full_path);
        close(dir_fd);
        return -1;
    }
    if (fchdir(dir_fd) != 0) {
        frame_control(TITLE_WARNING, MSG_WARNING, ACCESS_DENIED_MSG, full_path);
        close(dir_fd);
        return -1;
    }
    close(dir_fd);
    char old_dir[PATH_MAX];
    SAFE_STRNCPY(old_dir, current_dir_buf, sizeof(old_dir));
    if (getcwd(current_dir_buf, buf_size) == NULL) {
        frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "getcwd failed after fchdir");
        char normalized_fallback[PATH_MAX];
        if (realpath(full_path, normalized_fallback) != NULL) {
            SAFE_STRNCPY(current_dir_buf, normalized_fallback, buf_size);
        } else {
            SAFE_STRNCPY(current_dir_buf, old_dir, buf_size);
            char *display_dir = basename(old_dir);
            frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "Fallback to old dir: %s", display_dir);
        }
    }
    update_file_list();
    return 0;
}

int scan_directory(const char *dir_path,
                   unsigned int category_filter,
                   void **entries_out,
                   int *count_out) {
    return scan_directory_ex(dir_path, category_filter,
                            OUTPUT_MODE_FILE_LIST, entries_out, count_out);
}

void get_bottom_panel_geometry(WINDOW *win, int *field_y, int *actual_width) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    *actual_width = (max_x < FILE_LIST_FIXED_WIDTH) ? max_x : FILE_LIST_FIXED_WIDTH;
    *field_y = max_y - 3;
}
