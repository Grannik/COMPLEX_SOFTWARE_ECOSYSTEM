#define _GNU_SOURCE
#include <alsa/asoundlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

#include "../libstrprepare/libstrprepare.h"
#include "../libline/libline.h"
#include "../libtermcontrol/libtermcontrol.h"
#include "../common.h"
#include "pcm_core.h"

typedef enum {
    MSG_STATUS_B,
    MSG_WARNING_B,
    MSG_ERROR_B,
    MSG_INFO_B
} MessageTypeB;

typedef enum {
    MSG_LIFETIME_FIXED,
    MSG_LIFETIME_AUDIO,
    MSG_LIFETIME_PINNED,
    MSG_LIFETIME_UI_DEPENDENT,
    MSG_LIFETIME_MANUAL
} msg_lifetime_t;

typedef struct {
    msg_lifetime_t type;
    int fixed_timeout_sec;
    bool wait_audio;
    bool pinned;
    bool (*ui_condition)(void);
} msg_policy_t;

typedef struct {
    bool visible;
    time_t show_time;
    msg_policy_t policy;
    MessageTypeB type;
    char text[256];
} message_t;

static message_t current_message;
static bool pause_active = false;
int path_global = 70;

int time_elapsed(void);
bool audio_is_playing(void);
void show_message(MessageTypeB type, msg_policy_t policy, const char *text);
void show_fixed_message(MessageTypeB type, const char *text, int seconds);
void show_audio_message(MessageTypeB type, const char *text);
void show_pinned_message(MessageTypeB type, const char *text);
void message_init(void);
void hide_message(void);
void show_manual_message(MessageTypeB type, const char *text);
void msg_update_standalone(void);
void message_init(void);
void toggle_pause(void);
void hide_message(void);
void draw_filelist_standalone(WINDOW *win);
static void draw_title_and_message(WINDOW *win);
static void draw_progress_panel(WINDOW *win);

void toggle_pause(void)
{
    SAFE_MUTEX_LOCK(&player_control.mutex);

    if (!player_control.current_file) {
        pthread_mutex_unlock(&player_control.mutex);
        return;
    }
    bool now_paused = !player_control.paused;
    player_control.paused = now_paused;
    pause_active = now_paused;
    if (now_paused) {
        player_control.fading_out = 1;
        player_control.current_fade = FADE_STEPS;
        player_control.is_silent = 0;
        show_manual_message(MSG_STATUS_B, "PAUSE");
    } else {
        player_control.is_silent = 0;
        player_control.fading_in = 1;
        player_control.current_fade = 0;
        player_control.fading_out = 0;
        hide_message();
    }
    pthread_cond_signal(&player_control.cond);
    pthread_mutex_unlock(&player_control.mutex);
}

void show_manual_message(MessageTypeB type, const char *text)
{
    msg_policy_t policy = {
        .type = MSG_LIFETIME_MANUAL,
        .fixed_timeout_sec = 0,
        .wait_audio = false,
        .pinned = true,
        .ui_condition = NULL
    };
    show_message(type, policy, text);
}

void message_init(void)
{
    current_message.visible = false;
    current_message.show_time = 0;
}

void hide_message(void)
{
    current_message.visible = false;
    current_message.text[0] = '\0';
    current_message.show_time = 0;
}

bool audio_is_playing(void)
{
    bool playing;
    SAFE_MUTEX_LOCK(&player_control.mutex);
    playing = (!player_control.stop &&
               player_control.filename != NULL &&
               !player_control.paused);
    pthread_mutex_unlock(&player_control.mutex);
    return playing;
}

void show_message(MessageTypeB type, msg_policy_t policy, const char *text)
{
    if (!text)
        return;
    memset(&current_message, 0, sizeof(current_message));
    current_message.visible = true;
    current_message.show_time = time(NULL);
    current_message.type = type;
    current_message.policy = policy;
    strncpy(current_message.text,
            text,
            sizeof(current_message.text) - 1);
    current_message.text[sizeof(current_message.text) - 1] = '\0';
}

void show_fixed_message(MessageTypeB type, const char *text, int seconds)
{
    msg_policy_t policy = {
        .type = MSG_LIFETIME_FIXED,
        .fixed_timeout_sec = seconds,
        .wait_audio = false,
        .pinned = false,
        .ui_condition = NULL
    };
    show_message(type, policy, text);
}

void show_audio_message(MessageTypeB type, const char *text)
{
    msg_policy_t policy = {
        .type = MSG_LIFETIME_AUDIO,
        .fixed_timeout_sec = 0,
        .wait_audio = true,
        .pinned = false,
        .ui_condition = NULL
    };
    show_message(type, policy, text);
}

void show_pinned_message(MessageTypeB type, const char *text)
{
    msg_policy_t policy = {
        .type = MSG_LIFETIME_PINNED,
        .fixed_timeout_sec = 0,
        .wait_audio = false,
        .pinned = true,
        .ui_condition = NULL
    };
    show_message(type, policy, text);
}

void msg_update_standalone(void) {
    if (audio_is_playing()) {
        SAFE_MUTEX_LOCK(&player_control.mutex);
        const char *playing_file = player_control.current_filename;
        char playing_name[256] = {0};
        if (playing_file && playing_file[0]) {
            char temp_path[PATH_MAX];
            strncpy(temp_path, playing_file, sizeof(temp_path) - 1);
            temp_path[sizeof(temp_path) - 1] = '\0';
            char *base = basename(temp_path);
            strncpy(playing_name, base, sizeof(playing_name) - 1);
            playing_name[sizeof(playing_name) - 1] = '\0';
        }
        pthread_mutex_unlock(&player_control.mutex);
        if (playing_name[0] != '\0') {
            char msg_text[256];
            snprintf(msg_text, sizeof(msg_text), "Playback started for: %.228s", playing_name);
            if (!current_message.visible ||
                (current_message.policy.type != MSG_LIFETIME_MANUAL &&
                 current_message.policy.type != MSG_LIFETIME_PINNED &&
                 (strcmp(current_message.text, msg_text) != 0 ||
                  current_message.policy.type != MSG_LIFETIME_AUDIO))) {
                show_audio_message(MSG_STATUS_B, msg_text);
                return;
            }
        }
    } else if (current_message.visible && current_message.policy.type == MSG_LIFETIME_AUDIO) {
        hide_message();
    }
    if (!current_message.visible)
        return;
    msg_policy_t *p = &current_message.policy;
	switch (p->type) {
	case MSG_LIFETIME_FIXED:
	    if ((time(NULL) - current_message.show_time) >= p->fixed_timeout_sec)
	        hide_message();
	    break;
	case MSG_LIFETIME_AUDIO:
	    if (!audio_is_playing())
	        hide_message();
	    break;
	case MSG_LIFETIME_PINNED:
	    break;
	case MSG_LIFETIME_UI_DEPENDENT:
	    if (p->ui_condition && p->ui_condition())
	        hide_message();
	    break;
	case MSG_LIFETIME_MANUAL:
	    break;
	default:
	    break;
	    }
}

static inline void refresh_ui(void)
{
    draw_filelist_standalone(list_win);
    wnoutrefresh(list_win);
    doupdate();
}

void draw_title_and_message(WINDOW *win) {
    int field_y, actual_width;
    get_bottom_panel_geometry(win, &field_y, &actual_width);
    const char *title;
    const char *global_msg = current_message.text;
    int has_global_msg = current_message.visible;
    if (show_error) {
        title = "ERROR:";
    } else if (has_global_msg && current_message.type == MSG_WARNING_B) {
        title = "WARNING:";
    } else {
        title = "NOTIFICATION:";
    }
       win_msg_ncurses(win, 0, 3, actual_width, 1, 0, 5, -1, -1, 0, 0, 2, title);
    if (show_error) {
        wchar_t werror[256];
        prepare_display_wstring(error_msg, actual_width - 12, werror, sizeof(werror)/sizeof(wchar_t), 0, L"..", 0, 0);
        wattron(win, COLOR_PAIR(COLOR_PAIR_RED));
        mvwaddwstr(win, 2, 8, werror);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_RED));
    } else if (has_global_msg) {
        MessageTypeB msg_type = current_message.type;
        int color_pair = COLOR_PAIR_WHITE;
if (msg_type == MSG_WARNING_B)
    color_pair = COLOR_PAIR_RED;
else if (msg_type == MSG_INFO_B)
    color_pair = COLOR_PAIR_YELLOW;
        wattron(win, COLOR_PAIR(color_pair));
        wchar_t wtext[256];
        prepare_display_wstring(global_msg, actual_width - 18, wtext, sizeof(wtext)/sizeof(wchar_t), 0, L"..", 0, 0);
        mvwaddwstr(win, 2, 15, wtext);
        wattroff(win, COLOR_PAIR(color_pair));
    } else if (show_status) {
        int status_width = actual_width - 4;
        wchar_t wstatus[256];
        prepare_display_wstring(status_msg, status_width - 12, wstatus, sizeof(wstatus)/sizeof(wchar_t), 0, L"..", 0, 0);
        mvwaddwstr(win, 2, 15, wstatus);
    }
}

void draw_progress_panel(WINDOW *win) {
    int field_y, actual_width;
    get_bottom_panel_geometry(win, &field_y, &actual_width);
    win_msg_ncurses(win, 0, 4, actual_width, 3, 0, 5, -1, -1, 0, 0, 2, "PLAYBACK TIME:");
    win_msg_ncurses(win, 0, 5, actual_width, 3, 0, 5, -1, -1, 0, 0, 2, "PROGRESS:");
    SAFE_MUTEX_LOCK(&player_control.mutex);
    double duration = player_control.duration;
    long long bytes_read = player_control.bytes_read;
    pthread_mutex_unlock(&player_control.mutex);
    if (duration > 0.0) {
        long long total_bytes = (long long)(duration * 176400.0);
        double percent = total_bytes > 0 ? ((double)bytes_read * 100.0) / (double)total_bytes : 0.0;
        int elapsed_sec = (int)((double)bytes_read / 176400.0);
        int total_sec = (int)duration;
        print_formatted_time(win, 3, 16, elapsed_sec);
        mvwprintw(win, 3, 25, "/");
        print_formatted_time(win, 3, 27, total_sec);
        mvwprintw(win, 4, 15, "%3d%%", (int)(percent + 0.5));
        draw_progress_bar(win, 4, 21, percent, 50);
    } else {
        print_formatted_time(win, 3, 16, -1);
        mvwprintw(win, 3, 25, "/");
        print_formatted_time(win, 3, 27, -1);
        mvwprintw(win, 4, 15, "  0%%");
        draw_progress_bar(win, 4, 21, 0.0, 50);
    }
}

void draw_filelist_standalone(WINDOW *win) {
    if (!win) return;
       path_form(win, 1, 1, curr_dir, &path_visual, path_global, 0);
       wmove(win, 2, 0);
       wclrtoeol(win);
    if (show_error || show_status || current_message.visible) {
       wmove(win, 2, 0);
       wclrtoeol(win);
       draw_title_and_message(win);
    }
       draw_progress_panel(win);
       wnoutrefresh(win);
}

static int navigate_and_play_standalone(void) {
if (terminal_control(1) != 0) {
    msg_system_init();
    generic_msg_ncurses(1, 1, 1, 31, -1, "FATAL: Failed to initialize terminal");
    return -1;
}
    list_win = newwin(termheight - 4, INNER_WIDTH, 1, 1);
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
    draw_filelist_standalone(list_win);
    refresh();
int ch;
	while (1) {
	    ch = wgetch(list_win);
            msg_update_standalone();
if (show_status && (time(NULL) - status_start_time >= STATUS_DURATION_SECONDS)) {
    show_status = 0;
    status_msg[0] = '\0';
    draw_filelist_standalone(list_win);
    msg_update_standalone();
}
if (ch == ERR) {
    refresh_ui();
    continue;
}
	if (ch != 10) {
	    show_error = 0;
	    error_msg[0] = '\0';
	    show_status = 0;
	    status_msg[0] = '\0';
	}
    strcpy(status_msg, "");
    if (ch == 'q' || ch == 'Q') break;
       int should_exit = 0;
    if (handle_common_navigationkeys(ch, &selected_index, file_cnt, curr_dir, &path_visual, &should_exit, path_global)) {
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
    toggle_pause();
    break;
        case 10:
	            if (file_cnt > 0 && selected_index >= 0 && file_list && file_list[selected_index].name) {
                if (!file_list[selected_index].is_dir) {
                if (is_file_type(file_list[selected_index].name, FILE_TYPE_AUDIO_PCM)) {
                        char *full_path = xasprintf("%s/%s", curr_dir, file_list[selected_index].name);
                        if (!full_path) {
                            show_fixed_message(MSG_INFO_B, "Out of memory! Cannot play file.", 3);
                            break;
                        }
                        start_playback(full_path, file_list[selected_index].name, 0);
                        free(full_path);
                    } else {
                        show_fixed_message(MSG_ERROR_B, "Unsupported file type (not a .pcm audio file)", 3);
                    }
} else {
    if (change_directory(file_list[selected_index].name, curr_dir, curr_dir, PATH_MAX) == 0) {
    }
}
	            }
	            break;
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
    endwin();
    free_file_list();
    navigate_history(HISTORY_CLEAR, NULL);
    return 0;
}

int main(int argc, char *argv[]) {
        message_init();
        if (argc > 1 && argv[1] != NULL) {
        struct stat st;
        if (stat(argv[1], &st) != 0) {
char buf[256];
snprintf(buf, sizeof(buf), "Cannot access argument: %s", argv[1]);
show_fixed_message(MSG_ERROR_B, buf, 3);
            return -1;
        }
        char resolved[PATH_MAX];
        if (realpath(argv[1], resolved) == NULL) {
char buf[256];
snprintf(buf, sizeof(buf), "Cannot resolve path %s", argv[1]);
show_fixed_message(MSG_ERROR_B, buf, 3);
            return -1;
        }
        if (S_ISDIR(st.st_mode)) {
            if (chdir(resolved) != 0) {
char buf[256];
char resolved_short[128];
snprintf(resolved_short, sizeof(resolved_short), "%.120s", resolved);
snprintf(buf, sizeof(buf), "Failed to change to directory: %s", resolved_short);
show_fixed_message(MSG_ERROR_B, buf, 3);
                return -1;
            }
            load_playlist(".", &player_control);
            SAFE_MUTEX_LOCK(&player_control.mutex);
            player_control.stop = 0;
            player_control.paused = 0;
            player_control.current_track = 0;
            player_control.playlist_mode = 1;
            if (player_control.playlist && player_control.playlist_size > 0) {
                SAFE_FREE(player_control.filename);
                player_control.filename = strdup(player_control.playlist[0]);
            }
            pthread_cond_signal(&player_control.cond);
            pthread_mutex_unlock(&player_control.mutex);
            play_single_file();
            fade_in_on_start();
            char buf[256];
            char base_buf2[PATH_MAX];
            strncpy(base_buf2, resolved, sizeof(base_buf2) - 1);
            base_buf2[sizeof(base_buf2) - 1] = '\0';
            char *file_name_local = basename(base_buf2);
            snprintf(buf, sizeof(buf), "Playback started for file: %s", file_name_local);
            show_fixed_message(MSG_STATUS_B, buf, 2);
        } else if (S_ISREG(st.st_mode)) {
            char dir_buf[PATH_MAX];
            strncpy(dir_buf, resolved, sizeof(dir_buf) - 1);
            dir_buf[sizeof(dir_buf) - 1] = '\0';
            char *dir_name = dirname(dir_buf);
            char base_buf[PATH_MAX];
            strncpy(base_buf, resolved, sizeof(base_buf) - 1);
            base_buf[sizeof(base_buf) - 1] = '\0';
            char *file_name = basename(base_buf);
            if (chdir(dir_name) != 0) {
                display_message(ERROR, "Failed to change to file directory");
                return -1;
            }
            start_playback(resolved, file_name, 0);
            display_message(STATUS, "Playback started for file: %s", file_name);
        } else {
	char buf[256];
	snprintf(buf, sizeof(buf), "Not a file or directory: %s", argv[1]);
	show_fixed_message(MSG_ERROR_B, buf, 3);
            return -1;
        }
    }
    return run_player_system(navigate_and_play_standalone);
}

// clear;gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition -Wcast-qual -Wwrite-strings -Wformat=2 -Wnull-dereference -Wdouble-promotion -fsanitize=address,undefined -g -o pcm_player module_14_standalone.c ../libstrprepare/libstrprepare.c ../libline/libline.c ../libtermcolor/libtermcolor.c ../libtermcolor/pseudographic.c ../libtermcolor/tc_file_directories_color.c ../libtermcolor/tc_frame.c ../libtermcontrol/libtermcontrol.c ../common.c pcm_core.c -lncursesw -lasound -lpthread -lm
