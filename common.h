#ifndef COMMON_H
#define COMMON_H
#include <wchar.h>
#include <ncursesw/ncurses.h>
#include <time.h>
#include <stddef.h>

#define INNER_WIDTH 82
#define CURSOR_WIDTH 77
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define TITLE_KEEP ((TitleType)-1)
#define COLOR_KEEP ((MessageType)-1)
#define TEXT_KEEP  NULL
#define ACCESS_DENIED_MSG "Access denied to: %s"
#define SAFE_STRNCPY(dest, src, size) do { strncpy((dest), (src), (size)); (dest)[(size)-1] = '\0'; } while (0)
#define STATUS_DURATION_SECONDS 5
#define COLOR_PAIR_BORDER 1

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

int msg_is_visible(void);
int universal_get_block_size(int module_id);
int terminal_control(int mode);
int terminal_check_size(int min_width, int min_height);
int universal_get_current_line(int module_id);
int handle_scroll_keys(int ch, int *scroll_offset, int total_lines, int visible_lines);
int universal_get_total(int module_id);

void display_content_with_scroll(WINDOW *win, int start_y, int start_x, int height, int width, const char **content, int total_lines, int *scroll_offset);
void common_scroll(int direction, int *current_line, int block_size, int total);
void universal_scroll(int module_id, int direction);
void msg_update_timeouts(void);
void universal_set_current_line(int module_id, int line);
void universal_set_block_size(int module_id, int size);

typedef enum {
    DISPLAY_LINES,
    DISPLAY_FUNCS
} display_mode_t;

 void display_from_line(int start_line, void *content, int content_size);
 void term_control_termios(int flag);
 int handle_common_input(char input_char, int *current_module);

typedef struct {
    int first_draw;
    void (*draw_frame)(void);
} Module;

void module_draw(Module modules[], int count, int index);
void module_first_draw(int *first_draw, void (*draw_frame)(void));

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

#define SCROLL_FILLED L'█'
#define SCROLL_EMPTY L'▒'

extern char curr_dir[PATH_MAX];

void handle_path_scroll(int ch, int *offset, const char *dir_path);
int navigate_history(HistoryAction action, const char *path);
void update_file_list(void);
int scan_directory_ex(const char *dir_path, unsigned int category_filter, OutputMode output_mode, void **entries_out, int *count_out);
void top(WINDOW *win, const char *dir_path, int *visual_offset);
void draw_file_list(WINDOW *win);
int prepare_file_list_display(WINDOW *win, int *out_max_y, int *out_max_x, int *out_visible_lines, int *out_start_index, int *out_end_index);
void draw_scrollbar(WINDOW *win, int start_y, int height, int total_files, int visible_lines, int start_index, int bar_x);
void clear_rect(WINDOW *win, int start_y, int end_y, int start_x, int end_x);
void draw_fill_line(WINDOW *win, int start_y, int start_x, int length, const void *symbol, int is_horizontal, int is_wide);
void set_color_scheme_flag(char flag);
char *xasprintf(const char *fmt, ...);
int handle_module_switch_key(int ch);
void free_entries(void **entries_ptr, int *count_ptr, OutputMode mode);
int navigate_to(const char *target_dir, bool refresh_screen);
int truncate_and_measure_wstring(const char *src, int max_visual_width, wchar_t *dest, size_t dest_size, bool is_directory, const wchar_t *ellipsis, int scroll_offset,
int *out_visual_width);

void center_title_on_frame(WINDOW *win, int y, int actual_width, const char *title, const char *title_left, const char *title_right);
int calculate_visual_width(const char *src);

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

void frame_control(TitleType title_type, MessageType msg_type, const char *fmt, ...)
    __attribute__((format(printf,3,4)));
extern WINDOW *list_win;
extern int show_status;
extern char status_msg[];
const char *msg_get_current_text(void);
MessageType msg_get_current_msg_type(void);
TitleType   msg_get_current_title_type(void);
void get_bottom_panel_geometry(WINDOW *win, int *field_y, int *actual_width);
const char *msg_get_title_text(TitleType type);
int handle_common_navigation_keys(int ch, int *p_selected_index, int file_cnt, char *curr_dir, int *path_visual, int *should_exit);
void msg_system_init(void);
void msg_clear(void);
 int change_directory(const char *dir_name, const char *base_path, char *current_dir_buf, size_t buf_size);
const char *get_last_forward(void);
int scan_directory(const char *dir_path, unsigned int flags, void **result, int *count);

#endif
