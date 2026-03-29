#ifndef TC_FRAME_H
#define TC_FRAME_H

typedef enum {
    FRAME_NUL     =  0,
    FRAME_SINGLE  =  1,
    FRAME_ROUNDED =  2,
    FRAME_HEAVY   =  3,
    FRAME_DOUBLE  =  4,
    FRAME_DOTS    =  5,
    FRAME_PLUS    =  6,
    FRAME_STAR    =  7,
    FRAME_A       =  8,
    FRAME_B       =  9,
    FRAME_C       = 10,
    FRAME_D       = 11,
    FRAME_E       = 12,
    FRAME_STYLE_COUNT
} FrameStyleType;

 typedef struct FrameStyle FrameStyle;
 const FrameStyle* get_frame_style(FrameStyleType type);
 void get_module_pos(char *symbol, int *pos);
 int calculate_title_start(int x, int width, int title_block_len, int title_align, int title_int);
 int prepare_frame_title_common(
     const char *fmt, va_list args, int width, int x, int title_align, int title_int, wchar_t **wtitle_out, int *title_width_out, int *title_start_out);
 int prepare_frame_title(const char *fmt, va_list args, int max_width, const wchar_t *ellipsis, int use_ellipsis, int visual_offset,
     wchar_t **wtitle_out, int *title_width_out, int *title_block_len_out);

void generic_frame_ncurses(
WINDOW *win, int x, int y, int width, int height, int attr, int fg, int bg, int bgsingl, int title_align, int title_int, FrameStyleType style_type, const char *fmt, ...);
 void generic_frame_ansi(
 int mod, int x, int y, int width, int height, int attr, int fg, int bg, int bgsingl, int title_align, int title_int, FrameStyleType style_type, const char *fmt, ...);

void module_strip_ansi(int mod, int x, int y, int attr, int frame_fg, int frame_bg, int text_fg, char symbol, const char *module_name);
void module_strip_ncurses(WINDOW *win, int x, int y, int attr, int frame_fg, int frame_bg, int text_fg, char symbol, const char *module_name);

void cycle_frame_ncurses(
    WINDOW *win,
    int x, int y,
    int width, int height,
    int attr,
    int fg,
    int bg,
    int bgsingl,
    int title_align,
    int title_int,
    FrameStyleType style_type,
    const char *fmt, ...);

#endif
