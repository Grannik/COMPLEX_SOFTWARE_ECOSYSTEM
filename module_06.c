#include "common.h"
#include "content_color/content_color_16.h"
#include "libtermcolor/libtermcolor.h"
#include "libtermcontrol/libtermcontrol.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

void module_06_draw_frame(void);
void module_06_draw_content(void);
 int module_06_run(void);
 int module_06_first_draw = 1;
 int mod = 0;

void module_strip_ansi(
    int mod,
    int x,
    int y,
    int attr,
    int frame_fg,
    int frame_bg,
    int text_fg,
    char symbol,
    const char *module_name);

void module_06_draw_frame(void)
{
int rows, cols;
    term_screen_get_size(&rows, &cols);
    generic_frame_ansi(16, 1, 1, 84, rows, 22, 32, 0, 1, 0, 2, 4, "GRANNIK | COMPLEX SOFTWARE ECOSYSTEM");
    module_strip_ansi(256, 1, 2, 0, 2, -1, 15, '6', "Basic ANSI Text Attributes and 16-Color Palette List");
}

void module_06_draw_content(void) {
display_from_line(content_current_line, (void *)content, TOTAL_CONTENT);
}

int module_06_run(void)
{
    module_first_draw(&module_06_first_draw, module_06_draw_frame);
    current_content_size = TOTAL_CONTENT;
    int rows, cols;
    term_screen_get_size(&rows, &cols);
    termheight = rows;
    int available_lines = termheight - 4;
    if (available_lines < 1) available_lines = 1;
    int total = TOTAL_CONTENT;
    int new_block_size = (available_lines >= total) ? total : available_lines;
    universal_set_block_size(6, new_block_size);
    universal_set_current_line(6, universal_get_current_line(6));
    content_current_line = universal_get_current_line(6);
    content_block_size = new_block_size;
    module_06_draw_content();
    return 0;
}
