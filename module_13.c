#define _GNU_SOURCE

#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/select.h>
#include <poll.h>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <dirent.h>
#include <wchar.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <libgen.h>
#include <time.h>
#include <fcntl.h>
#include <strings.h>
#include <math.h>
#include <stdbool.h>

#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"
#include "common.h"

static void free_filelist(void);
static void handle_selected_file(int selected_index);
static int navigate_and_play(void);
int module_13_run(void);

void draw_field_frame(WINDOW *win);
static int just_closed_view = 0;
static inline int should_skip_entry(const struct dirent *entry, int filter_raw) {
    return (entry->d_name[0] == '.') || (filter_raw && !is_file_category(entry->d_name, FILE_CATEGORY_TEXT));
}

static time_t notification_start_time = 0;
static inline void refresh_ui(void)
{
    draw_file_list(list_win);
    wnoutrefresh(stdscr);
    wnoutrefresh(list_win);
    doupdate();
}

static void flush_input_buffer(void) {
    if (!list_win) return;
    wtimeout(list_win, 0);
    int c;
    while ((c = wgetch(list_win)) != ERR) {
    }
    wtimeout(list_win, 50);
    wtimeout(stdscr, 0);
    while (wgetch(stdscr) != ERR) {}
    wtimeout(stdscr, 50);
}

static void view_text_file(const char *full_path, const char *file_name) {
    if (!full_path || !file_name) return;
    TitleType previous_title = msg_get_current_title_type();
    MessageType previous_msg = msg_get_current_msg_type();
    const char *previous_text = msg_get_current_text();
    char saved_text[256] = {0};
    if (previous_text) {
        strncpy(saved_text, previous_text, sizeof(saved_text)-1);
        saved_text[sizeof(saved_text)-1] = '\0';
    }
    static int horiz_char_offset = 0;
    FILE *f = fopen(full_path, "r");
    if (!f) {
        frame_control(TITLE_WARNING, MSG_WARNING, "Cannot open: %s", file_name);
        return;
    }
    wchar_t **wlines = NULL;
    int line_count = 0;
    int capacity = 100;
    wlines = malloc((size_t)capacity * sizeof(wchar_t*));
    size_t *line_lengths = malloc((size_t)capacity * sizeof(size_t));
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
        if (line_count >= capacity) {
            capacity *= 2;
            wlines = realloc(wlines, (size_t)capacity * sizeof(wchar_t*));
            line_lengths = realloc(line_lengths, (size_t)capacity * sizeof(size_t));
        }
        wchar_t *wstr = NULL;
        size_t wlen = 0;
        if (convert_to_wchar(buf, &wstr, &wlen) != 0) {
            wstr = malloc((len + 1) * sizeof(wchar_t));
            for (size_t i = 0; i < len; i++) {
                wstr[i] = (wchar_t)buf[i];
            }
            wstr[len] = L'\0';
            wlen = len;
        }
        wlines[line_count] = wstr;
        line_lengths[line_count] = wlen;
        line_count++;
    }
    fclose(f);
    if (line_count == 0) {
        free(wlines);
        free(line_lengths);
        frame_control(previous_title, previous_msg, "%s", saved_text);
        notification_start_time = time(NULL);
        return;
    }
    int max_cell_width = 0;
    for (int i = 0; i < line_count; i++) {
        int line_width = 0;
        for (size_t j = 0; j < line_lengths[i]; j++) {
            line_width += get_char_width(wlines[i][j]);
        }
        if (line_width > max_cell_width) {
            max_cell_width = line_width;
        }
    }
    int top_line = 0;
    int ch;
    int viewing = 1;
    while (viewing) {
        int rows, cols;
        getmaxyx(list_win, rows, cols);
        cycle_frame_ncurses(list_win,1,4,82,rows-6,0,2,-1,-1,1,0,2,"TEXT");
        clear_rect(list_win, 4, rows - 4, 1, cols - 1);
        int max_top = line_count - (rows - 8);
        if (max_top < 0) max_top = 0;
        if (top_line > max_top) top_line = max_top;
        if (top_line < 0) top_line = 0;
        int visible_cells = cols - 2;
        if (visible_cells < 0) visible_cells = 0;
        int max_horiz = max_cell_width - visible_cells;
        if (max_horiz < 0) max_horiz = 0;
        if (horiz_char_offset < 0) horiz_char_offset = 0;
        if (horiz_char_offset > max_horiz) horiz_char_offset = max_horiz;
        int visible = rows - 8;
        if (visible < 1) visible = 1;
        int y = 4;
        for (int i = 0; i < visible && (top_line + i) < line_count; i++) {
            wchar_t *wline = wlines[top_line + i];
            size_t line_chars = line_lengths[top_line + i];
            int chars_to_show = cols - 2;
            if (chars_to_show < 0) chars_to_show = 0;
            wchar_t space = L' ';
            for (int x = 0; x < chars_to_show; x++) {
                mvwaddnwstr(list_win, y, 1 + x, &space, 1);
            }
            if (line_chars == 0) {
                y++;
                continue;
            }
            size_t start_char = 0;
            int cell_offset = 0;
            while (start_char < line_chars && cell_offset < horiz_char_offset) {
                int char_width = get_char_width(wline[start_char]);
                if (cell_offset + char_width > horiz_char_offset) break;
                cell_offset += char_width;
                start_char++;
            }
            int current_cell = 0;
            size_t current_char = start_char;
            while (current_char < line_chars && current_cell < chars_to_show) {
                wchar_t current_wchar = wline[current_char];
                int char_width = get_char_width(current_wchar);
                if (current_cell + char_width > chars_to_show) break;
                   wattron(list_win, COLOR_PAIR(get_color_pair(253, -1)));
                   mvwaddnwstr(list_win, y, 1 + current_cell, &current_wchar, 1);
                   wattroff(list_win, COLOR_PAIR(get_color_pair(253, -1)));
                current_cell += char_width;
                current_char++;
            }
            y++;
        }
           clear_rect(list_win, rows - 2, rows - 1, 1, cols - 1);
           wattron(list_win, COLOR_PAIR(get_color_pair(7, -1)));
           mvwprintw(list_win, rows - 2, 2, "First line: ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(7, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wprintw(list_win, "%d", top_line + 1);
           wattroff(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(28, -1)));
           wprintw(list_win, " | ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(28, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(7, -1)));
           wprintw(list_win, "Last line: ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(7, -1)));
        int last_visible_line = top_line + visible;
        if (last_visible_line > line_count) last_visible_line = line_count;
           wattron(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wprintw(list_win, "%d", last_visible_line);
           wattroff(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(28, -1)));
           wprintw(list_win, " | ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(28, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(7, -1)));
           wprintw(list_win, "Total lines: ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(7, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wprintw(list_win, "%d", line_count);
           wattroff(list_win, COLOR_PAIR(get_color_pair(3, -1)));
           wattron(list_win, COLOR_PAIR(get_color_pair(28, -1)));
           wprintw(list_win, " | ");
           wattroff(list_win, COLOR_PAIR(get_color_pair(28, -1)));
        if (max_horiz > 0) {
            wattron(list_win, COLOR_PAIR(get_color_pair(7, -1)));
            wprintw(list_win, "<->: ");
            wattroff(list_win, COLOR_PAIR(get_color_pair(7, -1)));
            wattron(list_win, COLOR_PAIR(get_color_pair(3, -1)));
            wprintw(list_win, "%d", horiz_char_offset);
            wattroff(list_win, COLOR_PAIR(get_color_pair(3, -1)));
        }
        wnoutrefresh(list_win);
        doupdate();
        wtimeout(list_win, 50);
        ch = wgetch(list_win);
        switch (ch) {
            case 'Z':
                viewing = 0;
                just_closed_view = 1;
                flush_input_buffer();
                frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "File closed: %s", file_name);
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
            case KEY_LEFT:
                horiz_char_offset -= 1;
                if (horiz_char_offset < 0) horiz_char_offset = 0;
                break;
            case KEY_RIGHT:
                horiz_char_offset += 1;
                if (horiz_char_offset > max_horiz) horiz_char_offset = max_horiz;
                break;
            case KEY_HOME:
                horiz_char_offset = 0;
                break;
            case KEY_END:
                horiz_char_offset = max_horiz;
                break;
            case 'b':
            case 'B':
                horiz_char_offset -= visible_cells / 2;
                if (horiz_char_offset < 0) horiz_char_offset = 0;
                break;
            case 'w':
            case 'W':
                horiz_char_offset += visible_cells / 2;
                if (horiz_char_offset > max_horiz) horiz_char_offset = max_horiz;
                break;
        }
        draw_field_frame(list_win);
    }
    for (int i = 0; i < line_count; i++) {
        free(wlines[i]);
    }
    free(wlines);
    free(line_lengths);
    draw_file_list(list_win);
}

static int show_error = 0;

void draw_field_frame(WINDOW *win)
{
    if (!win) return;
    int field_y, actual_width;
    get_bottom_panel_geometry(win, &field_y, &actual_width);
    const char *text = msg_get_current_text();
    MessageType msg_type = msg_get_current_msg_type();
    TitleType title_type = msg_get_current_title_type();
    const char *title = msg_get_title_text(title_type);
    cycle_frame_ncurses(win, 1, field_y + 1, actual_width, 3, 0, 2, -1, -1, 1, 0, 2, title);
if (text && msg_is_visible()) {
    int fg_color;
    switch (msg_type) {
        case MSG_FATAL:
            fg_color = 9;
            break;
        case MSG_WARNING:
            fg_color = 197;
            break;
        case MSG_DEBUG:
            fg_color = 245;
            break;
        case MSG_NOTIFICATION:
            fg_color = 11;
            break;
        case MSG_INFO:
            fg_color = 14;
            break;
        case MSG_NORMAL:
        default:
            fg_color = 15;
            break;
    }
    int color_pair = get_color_pair(fg_color, -1);
    clear_rect(win, field_y + 1, field_y + 2, 2, actual_width - 2);
    wattron(win, COLOR_PAIR(color_pair));
    wchar_t wtext[256];
    prepare_display_wstring(text, actual_width - 4, wtext, sizeof(wtext)/sizeof(wchar_t), 0, L"..", 0, 0);
    mvwaddwstr(win, field_y + 1, 2, wtext);
    wattroff(win, COLOR_PAIR(color_pair));
} else {
    clear_rect(win, field_y + 1, field_y + 2, 2, actual_width - 2);
}
    wnoutrefresh(win);
}

void free_filelist(void) {
    if (file_list) {
       free_entries((void**)&file_list, &file_cnt, OUTPUT_MODE_FILE_LIST);
    }
    file_list = NULL;
    file_cnt = 0;
    selected_index = 0;
}

static void handle_selected_file(int idx) {
    if (file_cnt <= 0 || idx < 0 || !file_list || !file_list[idx].name) {
        return;
    }
    if (!file_list[idx].is_dir) {
    if (is_file_category(file_list[idx].name, FILE_CATEGORY_TEXT)) {
            char *full_path = xasprintf("%s/%s", curr_dir, file_list[idx].name);
            if (!full_path) {
                frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "Out of memory! Cannot open file.");
                return;
            }
            view_text_file(full_path, file_list[idx].name);
            free(full_path);
        } else {
            frame_control(TITLE_WARNING, MSG_WARNING, "Unsupported file type (not a text file)");
        }
    }
}

int navigate_and_play(void) {
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, 'D', "Module text viewer");
    list_win = newwin(termheight - 4, INNER_WIDTH, 3, 1);
    if (!list_win) { endwin(); return -1; }
    frame_control(TITLE_VIEW_INFO, MSG_NORMAL, " First line: | Last line: | Total lines: | <->:");
    wtimeout(list_win, 50);
    wtimeout(stdscr, 50);
    keypad(list_win, TRUE);

    if (getcwd(curr_dir, PATH_MAX) == NULL) {
        delwin(list_win); endwin(); return -1;
    }
    strncpy(root_dir, curr_dir, sizeof(root_dir) - 2);
    root_dir[sizeof(root_dir) - 2] = '\0';
    if (root_dir[0] != '\0' && root_dir[strlen(root_dir) - 1] != '/') {
        strcat(root_dir, "/");
    }
    navigate_history(HISTORY_CLEAR, NULL);
    update_file_list();
    draw_file_list(list_win);
    refresh();
    int ch;
    while (1) {
        msg_update_timeouts();
        ch = wgetch(list_win);
    int action = handle_module_switch_key(ch);
    if (action == -1 || action > 0 || ch == '0') {
        ungetch(ch);
        navigate_history(HISTORY_CLEAR, NULL);
        break;
if (forward_history) {
    for (size_t i = 0; i < forward_count; i++) {
        free(forward_history[i]);
    }
    free(forward_history);
    forward_history = NULL;
    forward_count = 0;
    forward_capacity = 0;
}
//
    }
        if (show_status && (time(NULL) - status_start_time >= STATUS_DURATION_SECONDS)) {
            show_status = 0;
            status_msg[0] = '\0';
            draw_file_list(list_win);
        }
        if (ch == ERR) {
            draw_file_list(list_win);
            continue;
        }
        if (ch != 10) {
            show_error = 0;
            show_status = 0;
            status_msg[0] = '\0';
            if (msg_is_visible()) {
                MessageType mt = msg_get_current_msg_type();
                TitleType tt = msg_get_current_title_type();
                if (!(tt == TITLE_VIEW_INFO && mt == MSG_NORMAL)) {
                    msg_clear();
                frame_control(TITLE_VIEW_INFO, MSG_NORMAL, " First line: | Last line: | Total lines: | <->:");
                }
            }
        }
        int should_exit = 0;
        if (handle_common_navigation_keys(ch, &selected_index, file_cnt, curr_dir, &path_visual, &should_exit)) {
            if (should_exit) break;
            refresh_ui();
            continue;
        }
        strcpy(status_msg, "");
        switch (ch) {
            case 10:
                frame_control(TITLE_VIEW_INFO, COLOR_KEEP, TEXT_KEEP);
                if (file_cnt > 0 && selected_index >= 0 && file_list && file_list[selected_index].name) {
                if (file_list[selected_index].is_dir) {
                if (change_directory(file_list[selected_index].name, curr_dir, curr_dir, PATH_MAX) != 0) {
    }
} else {
    handle_selected_file(selected_index);
}
                }
                break;
case 'c':
case 'C': {
    if (file_cnt > 0 && selected_index >= 0 && file_list &&
        !file_list[selected_index].is_dir &&
        is_file_category(file_list[selected_index].name, FILE_CATEGORY_TEXT)) {

        char *full_path = xasprintf("%s/%s", curr_dir, file_list[selected_index].name);
        if (!full_path) {
            frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "Out of memory!");
            break;
        }
        FILE *f = fopen(full_path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize_long = ftell(f);
            fseek(f, 0, SEEK_SET);

            if (fsize_long < 0) {
                fclose(f);
                free(full_path);
                frame_control(TITLE_WARNING, MSG_WARNING, "Failed to get file size: %s", 
                             file_list[selected_index].name);
                break;
            }
            size_t fsize = (size_t)fsize_long;
            char *buffer = malloc(fsize + 1);
            if (buffer) {
                fread(buffer, 1, fsize, f);
                buffer[fsize] = 0;
                FILE *pipe = popen("xclip -selection clipboard", "w");
                if (pipe) {
                    fwrite(buffer, 1, fsize, pipe);
                    pclose(pipe);
                    frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION,
                                 "Copied to clipboard: %s", file_list[selected_index].name);
                } else {
                    frame_control(TITLE_WARNING, MSG_WARNING, "Failed to open xclip pipe");
                }
                free(buffer);
            }
            fclose(f);
        } else {
            frame_control(TITLE_WARNING, MSG_WARNING, "Cannot open file: %s",
                         file_list[selected_index].name);
        }
        free(full_path);
    } else {
        frame_control(TITLE_NOTIFICATION, MSG_NOTIFICATION, "Select a text file to copy");
    }
    break;
}
        }
        refresh_ui();
        if (show_status && ch != ERR) {
            show_status = 0;
        }
    }
    if (list_win) {
        delwin(list_win);
        list_win = NULL;
    }
    endwin();
    free_filelist();
    navigate_history(HISTORY_CLEAR, NULL);
    return 0;
}

int module_13_run(void) {
    set_color_scheme_flag('a');
    msg_system_init();
    msg_clear();
    int result = navigate_and_play();
    return result;
}
