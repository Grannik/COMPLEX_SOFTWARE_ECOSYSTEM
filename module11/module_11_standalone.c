#include <stdio.h>
#include <stdlib.h>

#include "../libtermcolor/libtermcolor.h"

extern int term_height;

typedef struct {
    int w,h;
    int attr, fg, bg, bgsingl, title_align, title_int, style;
    const char *text;
    const char *label;
} FrameBlock;

int main(void)
{
system("clear");
    color_init();
    reset_ansi(2);

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
    int start_y = 2;
    int step_y  = 4;
    int cols = 2;
    int rows = (term_height - start_y + 1) / step_y;
    int blocks_per_page = rows * cols;
    int pages = (total_blocks + blocks_per_page - 1) / blocks_per_page;

    int current_page = 0;
    if (current_page >= pages)
        current_page = pages - 1;
    if (current_page < 0)
        current_page = 0;

    int mod = 256;
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

        generic_frame_ansi(
            mod, x, y,
            blocks[i].w, blocks[i].h,
            blocks[i].attr, blocks[i].fg,
            blocks[i].bg, blocks[i].bgsingl,
            blocks[i].title_align, blocks[i].title_int,
            blocks[i].style, blocks[i].text
        );

        x_y_ansi(x + 2, y + 1);
        mod_fg_ansi(mod, blocks[i].fg);
        mod_bg_ansi(mod, blocks[i].bgsingl);
        printf("%s", blocks[i].label);
    }
    printf("\n");
    reset_ansi(1);
    return 0;
}

// gcc -I. module_11_standalone.c ../libtermcolor/libtermcolor.c ../libtermcolor/tc_frame.c -o module_11_standalone -lncurses -no-pie
