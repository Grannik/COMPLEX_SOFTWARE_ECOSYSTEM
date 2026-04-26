#ifndef COMMON_H
#define COMMON_H
#include <wchar.h>
#include <ncursesw/ncurses.h>
#include <time.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>

extern bool user_moved_cursor;
extern int path_display_width;

#define ERROR 1
#define MIN_WIDTH 84
#define CURSOR_WIDTH 77
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)
#define TITLE_KEEP ((TitleType)-1)
#define COLOR_KEEP ((MessageType)-1)
#define TEXT_KEEP  NULL
#define ACCESS_DENIED_MSG "Access denied to: %s"
#define SAFE_STRNCPY(dest, src, size) do { strncpy((dest), (src), (size)); (dest)[(size)-1] = '\0'; } while (0)
#define FADE_STEPS 48
#define STATUS_DURATION_SECONDS 5
#define SCROLL_FILLED L'█'
#define SCROLL_EMPTY L'▒'
#define SAFE_MUTEX_LOCK(m) do { int ret = pthread_mutex_lock(m); if (ret != 0) { display_message(ERROR, "Mutex lock failed: %s", strerror(ret)); } } while (0)

void display_message(int type, const char *fmt, ...);

typedef enum {
    MODULE_NCURSES,
    MODULE_ANSI
} ModuleType;

typedef struct FileEntry {
    char *name;
    int is_dir;
} FileEntry;

typedef enum {
    OUTPUT_MODE_FILE_LIST,
    OUTPUT_MODE_PLAYLIST
} OutputMode;

typedef enum {
    DISPLAY_LINES,
    DISPLAY_FUNCS
} display_mode_t;

typedef struct {
    int first_draw;
    void (*draw_frame)(void);
} Module;

typedef struct {
    wchar_t *wstr;
    size_t wlen;
    int visual_width;
} StringMetrics;

typedef enum {
HISTORY_ADD_FORWARD,
HISTORY_GET_LAST,
HISTORY_CLEAR
} HistoryAction;

typedef enum {
    TITLE_PLAYBACK_TIME_PROGRESS,
    TITLE_NOTIFICATION,
    TITLE_VIEW_INFO,
    TITLE_WARNING,
    TITLE_DEBUG,
    TITLE_EMPTY,
    TITLE_INFO,
    TITLE_TEXT,
} TitleType;

typedef enum {
    MSG_FATAL,
    MSG_WARNING,
    MSG_DEBUG,
    MSG_NOTIFICATION,
    MSG_INFO,
    MSG_NORMAL
} MessageType;

extern char curr_dir[PATH_MAX];
extern time_t status_start_time;
extern FileEntry *file_list;
extern ModuleType current_mode;
extern char current_color_flag;
extern int termheight;
extern int termwidth;
extern int current_content_size;
extern int content_block_size;
extern int content_current_line;
extern int file_cnt;
extern WINDOW *list_win;
extern int term_width;
extern int selected_index;
extern int path_visual;
extern char root_dir[PATH_MAX];
extern char **forward_history;
extern size_t forward_count;
extern size_t forward_capacity;
extern int colors_initialized;
extern int show_status;
extern char status_msg[];

int msg_is_visible(void);
int universal_get_block_size(int module_id);
int terminal_check_size(int min_width, int min_height);
int universal_get_current_line(int module_id);
int handle_scroll_keys(int ch, int *scroll_offset, int total_lines, int visible_lines);
int universal_get_total(int module_id);
int prepare_file_list_display(WINDOW *win, int *out_max_y, int *out_max_x, int *out_visible_lines, int *out_start_index, int *out_end_index, int minus);
int file_entry_cmp(const void *a, const void *b);
int handle_common_input(char input_char, int *current_module);
int navigate_history(HistoryAction action, const char *path);
int scan_directory_ex(const char *dir_path, unsigned int category_filter, OutputMode output_mode, void **entries_out, int *count_out);
int handle_module_switch_key(int ch);
int navigate_to(const char *target_dir, bool refresh_screen);
int calculate_visual_width(const char *src);
int change_directory(const char *dir_name, const char *base_path, char *current_dir_buf, size_t buf_size);
int handle_common_navigationkeys(int ch, int *p_selected_index, int file_count, char *current_dir, int *path_visual_offset, int *should_exit, int path_vr);
int scan_directory(const char *dir_path, unsigned int flags, void **result, int *count);
int playlist_cmp(const void *a, const void *b);
void handle_pathscroll(int ch, int *offset, const char *dir_path, int max_path_width);
void display_content_with_scroll(WINDOW *win, int start_y, int start_x, int height, int width, const char **content, int total_lines, int *scroll_offset);
void common_scroll(int direction, int *current_line, int block_size, int total);
void universal_scroll(int module_id, int direction);
void msg_update_timeouts(void);
void universal_set_current_line(int module_id, int line);
void universal_set_block_size(int module_id, int size);
void display_from_line(int start_line, void *content, int content_size);
void term_control_termios(int flag);
void module_draw(Module modules[], int count, int index);
void module_first_draw(int *first_draw, void (*draw_frame)(void));
void handle_path_scroll(int ch, int *offset, const char *dir_path);
void update_file_list(void);
void path_form(WINDOW *win, int x, int y, const char *dir_path, int *visual_offset, int max_path_width, int mode);
void draw_file_list(WINDOW *win);
void draw_scrollbar(WINDOW *win, int start_y, int height, int total_files, int visible_lines, int start_index, int bar_x);
void clear_rect(WINDOW *win, int start_y, int end_y, int start_x, int end_x);
void draw_fill_line(WINDOW *win, int start_y, int start_x, int length, const void *symbol, int is_horizontal, int is_wide);
void set_color_scheme_flag(char flag);
void free_entries(void **entries_ptr, int *count_ptr, OutputMode mode);
void msg_system_init(void);
void msg_clear(void);
void free_names(void *entries, int count, int is_file_entry);
void get_bottom_panel_geometry(WINDOW *win, int *field_y, int *actual_width);
void frame_control(TitleType title_type, MessageType msg_type, const char *fmt, ...)
    __attribute__((format(printf,3,4)));

MessageType msg_get_current_msg_type(void);
TitleType   msg_get_current_title_type(void);
const char *msg_get_title_text(TitleType type);
const char *msg_get_current_text(void);
const char *get_last_forward(void);

typedef struct PlayerControl {
    char *filename;
    int pause;
    int stop;
    int quit;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    FILE *current_file;
    char *current_filename;
    int paused;
    char **playlist;
    int playlist_size;
    int playlist_capacity;
    int current_track;
    int playlist_mode;
    int    seek_delta;
    double duration;
    long long bytes_read;
    int is_silent;
    int fading_out;
    int fading_in;
    int current_fade;
    char *playlist_dir;
    int loop_mode;
} PlayerControl;
extern PlayerControl player_control;

#endif
