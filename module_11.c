#include <curses.h>
#include "common.h"
#include "libtermcontrol/libtermcontrol.h"
#include "libtermcolor/libtermcolor.h"

int module_11_run(void);
int module_11_handle_input(int ch);

typedef struct {
    int w,h;
    int attr, fg, bg, bgsingl, title_align, title_int, style;
    const char *text;
    const char *label;
} FrameBlock;

static int current_page = 0;

int module_11_run(void)
{
    FrameBlock blocks[] = {
        {38,3,0,7,8,8,1,0,   0, "FRAME", "x y w h 0 0 8 8 0 0  0"},
        {38,3,0,7,-1,-1,1,0, 1, "FRAME", "x y w h 0 0 0 0 0 0  1"},
        {38,3,0,7,-1,-1,1,0, 2, "FRAME", "x y w h 0 0 0 0 0 0  2"},
        {38,3,0,7,-1,-1,1,0, 3, "FRAME", "x y w h 0 0 0 0 0 0  3"},
        {38,3,0,7,-1,-1,1,0, 4, "FRAME", "x y w h 0 0 0 0 0 0  4"},
        {38,3,0,7,-1,-1,1,0, 5, "FRAME", "x y w h 0 0 0 0 0 0  5"},
        {38,3,0,7,-1,-1,1,0, 6, "FRAME", "x y w h 0 0 0 0 0 0  6"},
        {38,3,0,7,-1,-1,1,0, 7, "FRAME", "x y w h 0 0 0 0 0 0  7"},
        {38,3,0,7,-1,-1,1,0, 8, "FRAME", "x y w h 0 0 0 0 0 0  8"},
        {38,3,0,7,-1,-1,1,0, 9, "FRAME", "x y w h 0 0 0 0 0 0  9"},
        {38,3,0,7,-1,-1,1,0,10, "FRAME", "x y w h 0 0 0 0 0 0 10"},
        {38,3,0,7,-1,-1,1,0,11, "FRAME", "x y w h 0 0 0 0 0 0 11"},
        {38,3,0,7,-1,-1,1,0,12, "FRAME", "x y w h 0 0 0 0 0 0 12"},
        {38,3,0,7,-1,-1,0,0, 1, "FRAME", "x y w h 0 0 0 0 * 0  *"},
        {38,3,0,7,-1,-1,1,0, 1, "FRAME", "x y w h 0 0 0 0 1 0  *"},
        {38,3,0,7,-1,-1,2,0, 1, "FRAME", "x y w h 0 0 0 0 2 0  *"},
        {38,3,1,7,-1,-1,1,0, 1, "FRAME", "x y w h 1 0 0 0 0 0  *"},
        {38,3,0,8,-1,-1,1,0, 1, "FRAME", "x y w h 0 8 0 0 0 0  *"},
        {38,3,0,7, 8,-1,1,0, 1, "FRAME", "x y w h 0 0 8 0 0 0  *"},
        {38,3,0,7,-1, 8,1,0, 1, "FRAME", "x y w h 0 0 0 8 0 0  *"}
    };
    int total_blocks = (int)(sizeof(blocks) / sizeof(blocks[0]));
    int start_x = 4;
    int start_y = 5;
    int step_y  = 4;
    int cols = 2;
    int rows = (LINES - start_y + 1) / step_y;
    int blocks_per_page = rows * cols;
    int pages = (total_blocks + blocks_per_page - 1) / blocks_per_page;
    if (current_page >= pages)
        current_page = pages - 1;
    if (current_page < 0) {
        current_page = 0;
        }
        generic_frame_ncurses(stdscr, 1, 1, 84, termheight, 0, 2, -1, -1, 1, 0, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
        module_strip_ncurses(stdscr, 2, 2, 0, 2, -1, 15, 'B', "FRAME");
    int start = current_page * blocks_per_page;
    int end   = start + blocks_per_page;
    if (end > total_blocks)
        end = total_blocks;
    for (int i = start; i < end; i++) {
        int local = i - start;
        int col = local % cols;
        int row = local / cols;
        int x = start_x + col * 41;
        int y = start_y + row * step_y;
        generic_frame_ncurses(
            stdscr, x, y,
            blocks[i].w, blocks[i].h,
            blocks[i].attr, blocks[i].fg,
            blocks[i].bg, blocks[i].bgsingl,
            blocks[i].title_align, blocks[i].title_int,
            blocks[i].style, blocks[i].text
        );
        wattron(stdscr, COLOR_PAIR(get_color_pair(blocks[i].fg, blocks[i].bgsingl)));
        mvprintw(y, x + 2, "%s", blocks[i].label);
        wattroff(stdscr, COLOR_PAIR(get_color_pair(blocks[i].fg, blocks[i].bgsingl)));
    }
    return 11;
}

int module_11_handle_input(int ch)
{
    int total_blocks = 20;
    int start_y = 5;
    int step_y  = 4;
    int cols = 2;
    int rows = (LINES - start_y + 1) / step_y;
    int blocks_per_page = rows * cols;
    int pages = (total_blocks + blocks_per_page - 1) / blocks_per_page;
    switch(ch) {
        case KEY_RIGHT:
            if (current_page < pages - 1)
                current_page++;
            return 1;
        case KEY_LEFT:
            if (current_page > 0)
                current_page--;
            return 1;
    }
    return 0;
}
