#include "libtermcontrol.h"
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <termios.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <term.h>
#include <signal.h>

#define PASTE_START "\033[200~"
#define PASTE_END   "\033[201~"

#include "../libtermcolor/libtermcolor.h"

static struct termios term_saved_state;

void term_init_context(void)
{
    tcgetattr(STDIN_FILENO, &term_saved_state);
}

void term_shutdown_restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_saved_state);
}

void term_init(int flags)
{
    if (flags & TERM_INIT_CONTEXT)
        term_init_context();

    if (flags & TERM_SHUTDOWN_RESTORE_TERMINAL)
        term_shutdown_restore_terminal();
}

static struct termios term_saved_state;
static struct termios term_current_state;

void term_termios_save_state(void)
{
    tcgetattr(STDIN_FILENO, &term_saved_state);
}

void term_termios_restore_state(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &term_saved_state);
}

void term_termios_enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_lflag &= ~((tcflag_t)(ICANON | ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios_enable_canonical_mode(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_lflag |= ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios_enable_echo(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios_disable_echo(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_lflag &= ~((tcflag_t)ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios_set_blocking_input(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_cc[VMIN]  = 1;
    term_current_state.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios_set_nonblocking_input(void)
{
    tcgetattr(STDIN_FILENO, &term_current_state);
    term_current_state.c_cc[VMIN]  = 0;
    term_current_state.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term_current_state);
}

void term_termios(
    int save_state,
    int restore_state,
    int enable_raw_mode,
    int enable_canonical_mode,
    int enable_echo,
    int disable_echo,
    int set_blocking_input,
    int set_nonblocking_input)
{
    if (save_state)
        term_termios_save_state();

    if (restore_state)
        term_termios_restore_state();

    if (enable_raw_mode)
        term_termios_enable_raw_mode();

    if (enable_canonical_mode)
        term_termios_enable_canonical_mode();

    if (enable_echo)
        term_termios_enable_echo();

    if (disable_echo)
        term_termios_disable_echo();

    if (set_blocking_input)
        term_termios_set_blocking_input();

    if (set_nonblocking_input)
        term_termios_set_nonblocking_input();
}

int term_input_available(void)
{
    int bytes = 0;
    ioctl(STDIN_FILENO, FIONREAD, &bytes);
    return bytes;
}

int term_input_read_key(void)
{
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return c;
    return -1;
}

int term_input_read_key_parsed(void)
{
    unsigned char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
        return -1;
    if (seq[0] != 27)
        return seq[0];
    read(STDIN_FILENO, &seq[1], 1);
    read(STDIN_FILENO, &seq[2], 1);
    return seq[2];
}

void term_input_read_line_basic(char *buffer, int size)
{
    fgets(buffer, size, stdin);
}

void term_input_flush_buffer(void)
{
    tcflush(STDIN_FILENO, TCIFLUSH);
}

void term_output_flush_buffer(void)
{
    fflush(stdout);
}

void term_input(
    int input_available,
    int read_key,
    int read_key_parsed,
    int read_line_basic,
    int input_flush_buffer,
    int output_flush_buffer)
{
    if (input_available)
        term_input_available();

    if (read_key)
        term_input_read_key();

    if (read_key_parsed)
        term_input_read_key_parsed();

    if (read_line_basic)
    {
        char buffer[1024];
        term_input_read_line_basic(buffer, sizeof(buffer));
    }

    if (input_flush_buffer)
        term_input_flush_buffer();

    if (output_flush_buffer)
        term_output_flush_buffer();
}

void term_screen_clear(void)
{
    printf("\033[2J");
    printf("\033[H");
}

void term_screen_clear_to_end(void)
{
    printf("\033[J");
}

void term_screen_scroll_up(void)
{
    printf("\033[S");
}

void term_screen_scroll_down(void)
{
    printf("\033[T");
}

void term_screen_get_size(int *rows, int *cols)
{
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    if (rows) *rows = ws.ws_row;
    if (cols) *cols = ws.ws_col;
}

void term_screen(
    int clear,
    int clear_to_end,
    int scroll_up,
    int scroll_down,
    int get_size)
{
    if (clear)
        term_screen_clear();

    if (clear_to_end)
        term_screen_clear_to_end();

    if (scroll_up)
        term_screen_scroll_up();

    if (scroll_down)
        term_screen_scroll_down();

    if (get_size)
    {
        int rows, cols;
        term_screen_get_size(&rows, &cols);
    }
}

void term_cursor_move(int row, int col)
{
    printf("\033[%d;%dH", row, col);
}

void term_cursor_move_relative(int row_offset, int col_offset)
{
    if (row_offset > 0)
        printf("\033[%dB", row_offset);
    else if (row_offset < 0)
        printf("\033[%dA", -row_offset);

    if (col_offset > 0)
        printf("\033[%dC", col_offset);
    else if (col_offset < 0)
        printf("\033[%dD", -col_offset);
}

void term_cursor_get_position(int *row, int *col)
{
    char buf[32];
    unsigned int i = 0;

    printf("\033[6n");
    fflush(stdout);

    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;

        if (buf[i] == 'R')
            break;

        i++;
    }

    buf[i] = '\0';

    if (buf[0] == '\033' && buf[1] == '[')
        sscanf(buf + 2, "%d;%d", row, col);
}

void term_cursor_hide(void)
{
    printf("\033[?25l");
}

void term_cursor_show(void)
{
    printf("\033[?25h");
}

void term_cursor_save(void)
{
    printf("\033[s");
}

void term_cursor_restore(void)
{
    printf("\033[u");
}

void term_cursor(
    int move,
    int move_relative,
    int get_position,
    int hide,
    int show,
    int save,
    int restore)
{
    if (move)
        term_cursor_move(1, 1);
    if (move_relative)
        term_cursor_move_relative(1, 1);
    if (get_position)
    {
        int row, col;
        term_cursor_get_position(&row, &col);
    }
    if (hide)
        term_cursor_hide();
    if (show)
        term_cursor_show();
    if (save)
        term_cursor_save();
    if (restore)
        term_cursor_restore();
}

static volatile sig_atomic_t resize_flag = 0;

static void term_resize_internal_handler(int sig)
{
    (void)sig;
    resize_flag = 1;
}

void term_resize_wait_event(void)
{
    resize_flag = 0;

    while (!resize_flag)
        pause();
}

void term_resize_register_sigwinch(void (*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler ? handler : term_resize_internal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGWINCH, &sa, NULL);
}

void term_resize(int wait_event, int register_sigwinch)
{
    if (register_sigwinch)
        term_resize_register_sigwinch(NULL);
    if (wait_event)
        term_resize_wait_event();
}

void term_screen_enable_alt_buffer(void)
{
    printf("\033[?1049h");
    fflush(stdout);
}

void term_screen_disable_alt_buffer(void)
{
    printf("\033[?1049l");
    fflush(stdout);
}

void term_screenbuf(
    int enable_alt_buffer,
    int disable_alt_buffer)
{
    if (enable_alt_buffer)
        term_screen_enable_alt_buffer();
    if (disable_alt_buffer)
        term_screen_disable_alt_buffer();
}

void term_mouse_enable_reporting(void)
{
    printf("\033[?1000h");
    printf("\033[?1006h");
    fflush(stdout);
}

void term_mouse_disable_reporting(void)
{
    printf("\033[?1000l");
    printf("\033[?1006l");
    fflush(stdout);
}

void term_mouse_read_event(void)
{
    char buffer[32];
        ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (n > 0)
        buffer[n] = '\0';
}

void term_mouse(
    int enable_reporting,
    int disable_reporting,
    int read_event)
{
    if (enable_reporting)
        term_mouse_enable_reporting();
    if (disable_reporting)
        term_mouse_disable_reporting();
    if (read_event)
        term_mouse_read_event();
}

void term_curses_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void term_curses_shutdown(void)
{
    endwin();
}

void term_curses_refresh(void)
{
    refresh();
}

void term_curses(
    int init,
    int shutdown,
    int refresh)
{
    if (init)
        term_curses_init();
    if (refresh)
        term_curses_refresh();
    if (shutdown)
        term_curses_shutdown();
}

typedef struct term_vbuffer_t
{
    int rows;
    int cols;
    char *cells;
    char *prev_cells;
} term_vbuffer_t;
static term_vbuffer_t vbuffer;

void term_vbuffer_create(int rows, int cols)
{
    vbuffer.rows = rows;
    vbuffer.cols = cols;
    vbuffer.cells = malloc((size_t)rows * (size_t)cols);
    vbuffer.prev_cells = malloc((size_t)rows * (size_t)cols);
    memset(vbuffer.cells, ' ', (size_t)rows * (size_t)cols);
    memset(vbuffer.prev_cells, ' ', (size_t)rows * (size_t)cols);
}

void term_vbuffer_destroy(void)
{
    if (vbuffer.cells)
        free(vbuffer.cells);
    if (vbuffer.prev_cells)
        free(vbuffer.prev_cells);
    vbuffer.cells = NULL;
    vbuffer.prev_cells = NULL;
}

void term_vbuffer_render(void)
{
    if (!vbuffer.cells)
        return;
    for (int r = 0; r < vbuffer.rows; r++)
    {
        printf("\033[%d;1H", r + 1);
        fwrite(
            &vbuffer.cells[r * vbuffer.cols],
            1,
            (size_t)vbuffer.cols,
            stdout);
    }
    fflush(stdout);
    memcpy(
        vbuffer.prev_cells,
        vbuffer.cells,
        (size_t)vbuffer.rows * (size_t)vbuffer.cols);
}

void term_vbuffer_update_diff(void)
{
    if (!vbuffer.cells || !vbuffer.prev_cells)
        return;
    for (int r = 0; r < vbuffer.rows; r++)
    {
        for (int c = 0; c < vbuffer.cols; c++)
        {
            int i = r * vbuffer.cols + c;
            if (vbuffer.cells[i] != vbuffer.prev_cells[i])
            {
                printf("\033[%d;%dH%c", r + 1, c + 1, vbuffer.cells[i]);
                vbuffer.prev_cells[i] = vbuffer.cells[i];
            }
        }
    }
    fflush(stdout);
}

void term_vbuffer(
    int create,
    int destroy,
    int render,
    int update_diff)
{
    if (create)
        term_vbuffer_create(24, 80);
    if (render)
        term_vbuffer_render();
    if (update_diff)
        term_vbuffer_update_diff();
    if (destroy)
        term_vbuffer_destroy();
}

void term_paste_enable_bracketed(void)
{
    printf("\033[?2004h");
    fflush(stdout);
}

void term_paste_disable_bracketed(void)
{
    printf("\033[?2004l");
    fflush(stdout);
}

int term_paste_read_block(char *buffer, int size)
{
    int i = 0;
    char c;
    while (i < size - 1)
    {
        if (read(STDIN_FILENO, &c, 1) != 1)
            break;
        buffer[i++] = c;
        if (i >= 6)
        {
            if (strncmp(&buffer[i - 6], PASTE_END, 6) == 0)
            {
                i -= 6;
                break;
            }
        }
    }

    buffer[i] = '\0';
    return i;
}

void term_paste(
    int enable_bracketed,
    int disable_bracketed,
    int read_block)
{
    if (enable_bracketed)
        term_paste_enable_bracketed();
    if (read_block)
    {
        char buffer[4096];
        term_paste_read_block(buffer, sizeof(buffer));
    }
    if (disable_bracketed)
        term_paste_disable_bracketed();
}

extern void term_shutdown_restore_terminal(void);
static void term_signal_cleanup_handler(int sig)
{
    term_shutdown_restore_terminal();
    printf("\n");
    fflush(stdout);
    signal(sig, SIG_DFL);
    raise(sig);
}

void term_signal_install_cleanup_handlers(void)
{
    signal(SIGINT,  term_signal_cleanup_handler);
    signal(SIGTERM, term_signal_cleanup_handler);
    signal(SIGQUIT, term_signal_cleanup_handler);
    signal(SIGHUP,  term_signal_cleanup_handler);
}

void term_signal_suspend_app(void)
{
    term_shutdown_restore_terminal();

    signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
}

void term_signal_resume_app(void)
{
    signal(SIGTSTP, SIG_IGN);
}

void term_signal(
    int install_cleanup_handlers,
    int suspend_app,
    int resume_app)
{
    if (install_cleanup_handlers)
        term_signal_install_cleanup_handlers();
    if (suspend_app)
        term_signal_suspend_app();
    if (resume_app)
        term_signal_resume_app();
}

static const char *term_detect_type(void)
{
    const char *term = getenv("TERM");
    if (!term)
        return "unknown";
    return term;
}

static void term_terminfo_query_capabilities(void)
{
    const char *term = term_detect_type();
    int err;
    if (setupterm(term, 1, &err) != OK)
    {
        printf("terminfo setup failed for terminal: %s\n", term);
        return;
    }
    int color_count = tigetnum("colors");
    int col_count = tigetnum("cols");
    int line_count = tigetnum("lines");
    char *cup_str = tigetstr("cup");
    char *clear_str = tigetstr("clear");
    printf("Terminal: %s\n", term);
    printf("Size: %d x %d\n", line_count, col_count);
    printf("Colors: %d\n", color_count);
    printf("Cursor movement: %s\n", cup_str ? "supported" : "not supported");
    printf("Clear screen: %s\n", clear_str ? "supported" : "not supported");
}

void term_termcap(
    int detect_type,
    int query_capabilities)
{
    if (detect_type)
    {
        const char *term = term_detect_type();
        printf("TERM=%s\n", term);
    }
    if (query_capabilities)
        term_terminfo_query_capabilities();
}

void term_attributes_reset(void) {
    printf("\033[0m");
    fflush(stdout);
}

void term_screen_clear_scrollback(void) {
    printf("\033[3J");
    fflush(stdout);
}

void term_screen_clear_and_home(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void term_screen_restore_full(void) {
    term_cursor_show();
    term_attributes_reset();
    term_screen_clear_scrollback();
    term_screen_clear_and_home();
    term_output_flush_buffer();
}

int terminal_check_size(int min_width, int min_height)
{
    if (isatty(STDOUT_FILENO)) {
        int rows, cols;
        term_screen_get_size(&rows, &cols);
        if (cols < min_width || rows < min_height) {
            reset_ansi(2);
            generic_msg_ansi(16, 1, 1, 1, 31, -1, " FATAL:");
            generic_msg_ansi(
                16, 9, 1, 3, 35, -1,
                "Terminal too small! Minimum: %dx%d",
                min_width, min_height
            );
            generic_msg_ansi(16, 1, 2, 1, 31, -1, " WARNING:");
            generic_msg_ansi(
                16, 11, 2, 3, 35, -1,
                "Current size: %dx%d",
                cols, rows
            );
            return -1;
        }
    }
    return 0;
}
