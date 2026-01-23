#include "common.h"
#include "content_help.h"
#include "libtermcolor/libtermcolor.h"
#include "content_var.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int module_00_run(void) {
    current_content_size = HELP_TOTAL_CONTENT;
    int term_height = get_terminal_height();
    int available_lines = term_height - 4;
    if (available_lines < 1) available_lines = 1;
    int total = HELP_TOTAL_CONTENT;
    int new_block_size = (available_lines >= total) ? total : available_lines;
    universal_set_block_size(0, new_block_size);
    universal_set_current_line(0, universal_get_current_line(0));
    content_block_size = new_block_size;
    content_current_line = universal_get_current_line(0);
    draw_exact_frame();
    libtermcolor_pos(2, 2, POS, 32,"____[", 37,"0", 32,"][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", NUL);
    libtermcolor_pos(6, 3, POS, 32,"Reference information. Module - usage", NUL);
    display_from_line_mode(content_current_line, DISPLAY_LINES, (void *)help_content, HELP_TOTAL_CONTENT);
    return 0;
}
