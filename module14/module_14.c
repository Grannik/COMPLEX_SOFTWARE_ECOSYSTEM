#define _GNU_SOURCE
#include <alsa/asoundlib.h>
#include <dirent.h>

#include "../libstrprepare/libstrprepare.h"
#include "../libline/libline.h"
#include "../common.h"
#include "pcm_core.h"

void draw_filelist_module(WINDOW *win);
int module_14_run(void);
static void draw_field_frame(WINDOW *win);

static inline int should_skip_entry(const struct dirent *entry, int filter_raw) {
return (entry->d_name[0] == '.') || (filter_raw && !is_file_type(entry->d_name, FILE_TYPE_AUDIO_PCM));
}

static inline void refresh_ui(void)
{
    draw_filelist_module(list_win);
    wnoutrefresh(stdscr);
    wnoutrefresh(list_win);
    doupdate();
}

static void draw_field_frame(WINDOW *win)
{
    int field_y, actual_width;
    get_bottom_panel_geometry(win, &field_y, &actual_width);
    const char *title;
    const char *global_msg = msg_get_current_text();
    int has_global_msg = msg_is_visible();
    if (show_error || show_status || has_global_msg) {
        title = "NOTIFICATION";
    } else {
        title = "PLAYBACK TIME & PROGRESS";
    }
    cycle_frame_ncurses(win, 1, field_y + 1, actual_width, 3, 0, 2, -1, -1, 1, 0, 2, title);
    if (show_error) {
        clear_rect(win, field_y + 1, field_y + 2, 2, actual_width - 2);
        wchar_t werror[256];
        prepare_display_wstring(error_msg, actual_width - 4, werror,
                              sizeof(werror)/sizeof(wchar_t), 0, L"..", 0, 0);
        wattron(win, COLOR_PAIR(COLOR_PAIR_RED));
        mvwaddwstr(win, field_y + 1, 2, werror);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_RED));
    } else if (has_global_msg) {
        clear_rect(win, field_y + 1, field_y + 2, 2, actual_width - 2);
        MessageType msg_type = msg_get_current_msg_type();
        int color_pair = COLOR_PAIR_WHITE;
        if (msg_type == MSG_WARNING) color_pair = COLOR_PAIR_RED;
        else if (msg_type == MSG_NOTIFICATION) color_pair = COLOR_PAIR_YELLOW;
        wattron(win, COLOR_PAIR(color_pair));
        wchar_t wtext[256];
        prepare_display_wstring(global_msg, actual_width - 4, wtext,
                              sizeof(wtext)/sizeof(wchar_t), 0, L"..", 0, 0);
        mvwaddwstr(win, field_y + 1, 2, wtext);
        wattroff(win, COLOR_PAIR(color_pair));
    } else {
        wattron(win, COLOR_PAIR(COLOR_PAIR_WHITE));
 if (show_status) {
            clear_rect(win, field_y + 1, field_y + 2, 2, actual_width - 2);
wattron(win, COLOR_PAIR(COLOR_PAIR_YELLOW));
int status_width = actual_width - 4;
wchar_t wstatus[256];
prepare_display_wstring(status_msg, status_width, wstatus, sizeof(wstatus)/sizeof(wchar_t), 0, L"..", 0, 0);
mvwaddwstr(win, field_y + 1, 2, wstatus);
wattroff(win, COLOR_PAIR(COLOR_PAIR_YELLOW));
        } else {
SAFE_MUTEX_LOCK(&player_control.mutex);
            double duration = player_control.duration;
long long bytes_read = player_control.bytes_read;
            pthread_mutex_unlock(&player_control.mutex);
            if (duration > 0.0) {
                  long long total_bytes = (long long)(duration * 176400.0);
                    double percent = total_bytes > 0 ? ((double)bytes_read * 100.0) / (double)total_bytes : 0.0;
                    int elapsed_sec = (int)((double)bytes_read / 176400.0);
                int total_sec = (int)duration;
                print_formatted_time(win, field_y + 1, 2, elapsed_sec);
                mvwprintw(win, field_y + 1, 11, "/");
                print_formatted_time(win, field_y + 1, 13, total_sec);
                wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                mvwprintw(win, field_y + 1, 22, "|");
                wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                mvwprintw(win, field_y + 1, 24, "%3d%%", (int)(percent + 0.5));
                wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                draw_progress_bar(win, field_y + 1, 30, percent, 50);
            } else {
                print_formatted_time(win, field_y + 1, 2, -1);
                mvwprintw(win, field_y + 1, 11, "/");
                print_formatted_time(win, field_y + 1, 13, -1);
                wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                mvwprintw(win, field_y + 1, 22, "|");
                wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                mvwprintw(win, field_y + 1, 24, "  0%%");
                wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));
                draw_progress_bar(win, field_y + 1, 30, 0.0, 50);
            }
        }
        wattroff(win, COLOR_PAIR(COLOR_PAIR_WHITE));
    }
}

void draw_filelist_module(WINDOW *win) {
    int max_y, max_x, visible_lines, start_index, end_index;
    int result = prepare_file_list_display(win, &max_y, &max_x, &visible_lines, &start_index, &end_index, 6);
    if (result <= 1) return;
    char *current_file_name = NULL;
    int current_paused = 0;
    SAFE_MUTEX_LOCK(&player_control.mutex);
    const char *active_filename = player_control.current_filename;
    if (!active_filename && player_control.filename) {
        active_filename = player_control.filename;
    }
    if (active_filename && strlen(active_filename) > 0) {
        const char *slash = strrchr(active_filename, '/');
        assign_safe_strdup(&current_file_name, slash ? slash + 1 : active_filename);
        if (current_file_name) {
            current_paused = player_control.paused;
        }
    }
    pthread_mutex_unlock(&player_control.mutex);
	if (file_cnt == 1 && file_list[0].name &&
	    strcmp(file_list[0].name, "Directory is empty or not enough memory.") == 0) {
	    int msg_row = 4 + (visible_lines / 2);
	    if (msg_row < 5) msg_row = 5;
	    if (msg_row >= max_y - 5) msg_row = max_y - 6;
            wattron(win, COLOR_PAIR(COLOR_PAIR_RED));
	    const char* msg = file_list[0].name;
            wchar_t wmsg[CURSOR_WIDTH + 4] = {0};
            prepare_display_wstring(msg, CURSOR_WIDTH, wmsg, sizeof(wmsg)/sizeof(wchar_t), 0, L"..", 0, 0);
            mvwaddwstr(win, msg_row, 3, wmsg);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_RED));
	    if (current_file_name) free(current_file_name);
	    draw_field_frame(win);
	    return;
	}
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
        draw_fill_line(win, row, 2, cursor_end - 2, &fill_ch1, 1, 1);
        wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
        int text_color = COLOR_PAIR_BORDER;
    if (!file_list[i].is_dir && is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM) &&
            current_file_name && strcmp(file_list[i].name, current_file_name) == 0) {
            text_color = current_paused ? COLOR_PAIR_YELLOW : COLOR_PAIR_BLUE;
        }
        else if (file_list[i].is_dir && player_control.playlist_mode &&
            player_control.playlist_dir)
        {
            char *full_folder_path = xasprintf("%s/%s", curr_dir, file_list[i].name);
            char resolved_path[PATH_MAX];
            char resolved_playlist_dir[PATH_MAX];
            if (full_folder_path &&
                realpath(full_folder_path, resolved_path) != NULL &&
                realpath(player_control.playlist_dir, resolved_playlist_dir) != NULL &&
                strcmp(resolved_path, resolved_playlist_dir) == 0) {
                text_color = COLOR_PAIR_BLUE;
            }
            free(full_folder_path);
        }
        wattron(win, COLOR_PAIR(text_color));
        mvwaddwstr(win, row, 3, wname);
        wattroff(win, COLOR_PAIR(text_color));
        wattron(win, COLOR_PAIR(get_color_pair(2, -1)));
        wchar_t fill_ch2 = L'▒';
        draw_fill_line(win, row, 3 + printed, cursor_end - (3 + printed), &fill_ch2, 1, 1);
        wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
        } else {
            int text_color = 0;
            if (player_control.playlist_mode &&
                player_control.playlist_dir &&
                file_list[i].is_dir)
            {
                char *full_folder_path = xasprintf("%s/%s", curr_dir, file_list[i].name);
                char resolved_path[PATH_MAX];
                char resolved_playlist_dir[PATH_MAX];
                if (full_folder_path &&
                    realpath(full_folder_path, resolved_path) != NULL &&
                    realpath(player_control.playlist_dir, resolved_playlist_dir) != NULL &&
                    strcmp(resolved_path, resolved_playlist_dir) == 0) {
                    text_color = COLOR_PAIR_BLUE;
                }
                free(full_folder_path);
            }
            if (text_color == 0) {
                if (current_file_name &&
                    strcmp(file_list[i].name, current_file_name) == 0 &&
                    is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM)) {
                    text_color = current_paused ? COLOR_PAIR_YELLOW : COLOR_PAIR_BLUE;
                } else {
                    text_color = get_text_module_file_color(file_list[i].name, file_list[i].is_dir, 'b');
                }
            }
            if (text_color) wattron(win, COLOR_PAIR(text_color));
               mvwaddwstr(win, row, 3, wname);
            if (text_color) wattroff(win, COLOR_PAIR(text_color));
		wchar_t space_ch = L' ';
		draw_fill_line(win, row, 3 + printed, cursor_end - (3 + printed), &space_ch, 1, 1);
	    }
}
    if (file_cnt > visible_lines) {
        int scroll_height       = max_y - 8;
        float ratio             = (float)scroll_height / (float)file_cnt;
        int scroll_bar_height   = (int)((float)visible_lines * ratio + 0.5f);
        if (scroll_bar_height < 1) scroll_bar_height = 1;
        if (scroll_bar_height > scroll_height) scroll_bar_height = scroll_height;
        int max_scroll_offset   = file_cnt - visible_lines;
        int scroll_pos          = 4;
        if (max_scroll_offset > 0) {
           float normalized_pos = (float)start_index / (float)max_scroll_offset;
           scroll_pos = 4 + (int)(normalized_pos * (float)(scroll_height - scroll_bar_height));
            if (scroll_pos < 4) scroll_pos = 4;
            if (scroll_pos > 4 + scroll_height - scroll_bar_height) scroll_pos = 4 + scroll_height - scroll_bar_height;
        }
        int bar_x = 2 + CURSOR_WIDTH + 1;
	wchar_t empty_char = SCROLL_EMPTY;
	wchar_t fill_char = SCROLL_FILLED;
	wattron(win, COLOR_PAIR(get_color_pair(2, -1)));
	draw_fill_line(win, 4, bar_x, scroll_height, &empty_char, 0, 1);
	draw_fill_line(win, scroll_pos, bar_x, scroll_bar_height, &fill_char, 0, 1);
	wattroff(win, COLOR_PAIR(get_color_pair(2, -1)));
    }
SAFE_FREE(current_file_name);
int last_list_row = end_index - start_index + 4;
int safe_clear_end = max_y - 5;
if (last_list_row < safe_clear_end) {
        clear_rect(win, last_list_row, safe_clear_end, 1, max_x - 1);
}
	draw_field_frame(win);
}

static int navigate_and_play(void) {
    generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, 'E', "Music pcm player module");
    list_win = newwin(termheight - 4, INNER_WIDTH, 3, 1);
    if (!list_win) { endwin(); return -1; }
    wtimeout(list_win, 50);
    wtimeout(stdscr, 50);
    keypad(list_win, TRUE);
    if (getcwd(curr_dir, PATH_MAX) == NULL) {
        delwin(list_win); endwin(); return -1;
    }
    forward_history = malloc((size_t)forward_capacity * sizeof(char *));
    if (!forward_history) {
        fprintf(stderr, "Error: not enough memory for navigation history — function disabled\n");
        forward_capacity = 0;
        forward_count = 0;
    } else {
        memset(forward_history, 0, (size_t)forward_capacity * sizeof(char *));
    }
    strncpy(root_dir, curr_dir, sizeof(root_dir) - 2);
    root_dir[sizeof(root_dir) - 2] = '\0';
    if (root_dir[0] != '\0' && root_dir[strlen(root_dir) - 1] != '/') {
        strcat(root_dir, "/");
    }
    navigate_history(HISTORY_CLEAR, NULL);
    update_filelist();
    draw_filelist_module(list_win);
    msg_clear();
    refresh();
    int ch;
    int next_module = 14;
    while (1) {
        ch = wgetch(list_win);
	int action = handle_module_switch_key(ch);
	if (action == -1 || action >= 0 || ch == '0') {
	    ungetch(ch);
	    break;
	}
        if (show_status && (time(NULL) - status_start_time >= STATUS_DURATION_SECONDS)) {
            show_status = 0;
            status_msg[0] = '\0';
            draw_filelist_module(list_win);
            msg_update_timeouts();
        }
        if (ch == ERR) {
            draw_filelist_module(list_win);
            continue;
        }
        if (ch != 10) {
            show_error = 0;
            error_msg[0] = '\0';
            show_status = 0;
            status_msg[0] = '\0';
            msg_clear();
        }
        strcpy(status_msg, "");
        int should_exit = 0;
        if (handle_common_navigationkeys(ch, &selected_index, file_cnt, curr_dir, &path_visual, &should_exit, 45)) {
            if (should_exit) break;
            refresh_ui();
            continue;
        }
        if (keys_play(ch)) {
            refresh_ui();
            continue;
        }
        switch (ch) {
            case ',':
                lock_and_signal(&player_control, action_p);
                break;
            case 10:
                if (file_cnt > 0 && selected_index >= 0 && file_list && file_list[selected_index].name) {
                    if (!file_list[selected_index].is_dir) {
                        if (is_file_type(file_list[selected_index].name, FILE_TYPE_AUDIO_PCM)) {
                            char *full_path = xasprintf("%s/%s", curr_dir, file_list[selected_index].name);
                            if (!full_path) {
                                display_message(STATUS, "Out of memory! Cannot play file.");
                                break;
                            }
                            start_playback(full_path, file_list[selected_index].name, 0);
                            free(full_path);
                        } else {
                            display_message(ERROR, "Unsupported file type (not a .pcm audio file)");
                        }
                    } else {
                        if (change_directory(file_list[selected_index].name, curr_dir, curr_dir, PATH_MAX) == 0) {
                        }
                    }
                }
                break;
        }
        if (show_status && ch != ERR) {
            show_status = 0;
        }
    }
    refresh_ui();
    if (show_status && ch != ERR) {
        show_status = 0;
    }
    if (list_win) {
        delwin(list_win);
        list_win = NULL;
    }
    free_file_list();
    navigate_history(HISTORY_CLEAR, NULL);
    nodelay(stdscr, FALSE);
    wtimeout(stdscr, -1);
    wtimeout(list_win, -1);
    return next_module;
}

int module_14_run(void) {
    return run_player_system(navigate_and_play);
}
