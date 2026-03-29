#ifndef COMMON_H
#define COMMON_H
#include <wchar.h>
#include <ncursesw/ncurses.h>

#define INNER_WIDTH 82
#define CURSOR_WIDTH 77
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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

int terminal_control(int mode);
int terminal_check_size(int min_width, int min_height);
void display_content_with_scroll(WINDOW *win, int start_y, int start_x, int height, int width, const char **content, int total_lines, int *scroll_offset);
int handle_scroll_keys(int ch, int *scroll_offset, int total_lines, int visible_lines);

void common_scroll(int direction, int *current_line, int block_size, int total);
void universal_scroll(int module_id, int direction);
int universal_get_current_line(int module_id);
void universal_set_current_line(int module_id, int line);
int universal_get_block_size(int module_id);
void universal_set_block_size(int module_id, int size);
int universal_get_total(int module_id);
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
int handle_common_navigation_keys(int ch, int *p_selected_index, int file_count, char *current_dir, int *path_visual_offset, int *should_exit);
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
int truncate_and_measure_wstring(const char *src, int max_visual_width, wchar_t *dest, size_t dest_size,
                                 bool is_directory, const wchar_t *ellipsis, int scroll_offset,
                                 int *out_visual_width);

#endif
