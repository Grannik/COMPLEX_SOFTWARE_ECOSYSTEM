#ifndef TERMINAL_CONTROL_H
#define TERMINAL_CONTROL_H

typedef enum
{
    TERM_INIT_CONTEXT              = 1 << 0,
    TERM_SHUTDOWN_RESTORE_TERMINAL = 1 << 1
} term_init_flags;

void term_init_context(void);
void term_shutdown_restore_terminal(void);
void term_init(int flags);

typedef enum
{
    TERM_TERMIOS_SAVE_STATE           = 1 << 0,
    TERM_TERMIOS_RESTORE_STATE        = 1 << 1,
    TERM_TERMIOS_ENABLE_RAW_MODE      = 1 << 2,
    TERM_TERMIOS_ENABLE_CANONICAL     = 1 << 3,
    TERM_TERMIOS_ENABLE_ECHO          = 1 << 4,
    TERM_TERMIOS_DISABLE_ECHO         = 1 << 5,
    TERM_TERMIOS_SET_BLOCKING         = 1 << 6,
    TERM_TERMIOS_SET_NONBLOCKING      = 1 << 7

} term_termios_flags;

void term_termios_save_state(void);
void term_termios_restore_state(void);

void term_termios_enable_raw_mode(void);
void term_termios_enable_canonical_mode(void);

void term_termios_enable_echo(void);
void term_termios_disable_echo(void);

void term_termios_set_blocking_input(void);
void term_termios_set_nonblocking_input(void);

void term_termios(
    int save_state,
    int restore_state,
    int enable_raw_mode,
    int enable_canonical_mode,
    int enable_echo,
    int disable_echo,
    int set_blocking_input,
    int set_nonblocking_input);

int  term_input_available(void);
int  term_input_read_key(void);
int  term_input_read_key_parsed(void);
void term_input_read_line_basic(char *buffer, int size);
void term_input_flush_buffer(void);
void term_output_flush_buffer(void);

void term_input(
    int input_available,
    int read_key,
    int read_key_parsed,
    int read_line_basic,
    int input_flush_buffer,
    int output_flush_buffer
);

void term_screen_clear(void);
void term_screen_clear_to_end(void);
void term_screen_scroll_up(void);
void term_screen_scroll_down(void);
void term_screen_get_size(int *rows, int *cols);

void term_screen(
    int clear,
    int clear_to_end,
    int scroll_up,
    int scroll_down,
    int get_size
);

void term_cursor_move(int row, int col);
void term_cursor_move_relative(int row_offset, int col_offset);
void term_cursor_get_position(int *row, int *col);
void term_cursor_hide(void);
void term_cursor_show(void);
void term_cursor_save(void);
void term_cursor_restore(void);

void term_cursor(
    int move,
    int move_relative,
    int get_position,
    int hide,
    int show,
    int save,
    int restore
);

void term_resize_wait_event(void);
void term_resize_register_sigwinch(void (*handler)(int));

void term_resize(
    int wait_event,
    int register_sigwinch
);

void term_screen_enable_alt_buffer(void);
void term_screen_disable_alt_buffer(void);

void term_screenbuf(
    int enable_alt_buffer,
    int disable_alt_buffer
);

void term_mouse_enable_reporting(void);
void term_mouse_disable_reporting(void);
void term_mouse_read_event(void);

void term_mouse(
    int enable_reporting,
    int disable_reporting,
    int read_event
);

void term_curses_init(void);
void term_curses_shutdown(void);
void term_curses_refresh(void);

void term_curses(
    int init,
    int shutdown,
    int refresh
);

void term_vbuffer_create(int rows, int cols);
void term_vbuffer_destroy(void);
void term_vbuffer_render(void);
void term_vbuffer_update_diff(void);

void term_vbuffer(
    int create,
    int destroy,
    int render,
    int update_diff);

void term_paste_enable_bracketed(void);
void term_paste_disable_bracketed(void);
int term_paste_read_block(char *buffer, int size);

void term_paste(
    int enable_bracketed,
    int disable_bracketed,
    int read_block);

void term_signal_install_cleanup_handlers(void);
void term_signal_suspend_app(void);
void term_signal_resume_app(void);

void term_signal(
    int install_cleanup_handlers,
    int suspend_app,
    int resume_app);

void term_termcap(
    int detect_type,
    int query_capabilities);

void term_attributes_reset(void);

void term_screen_clear_scrollback(void);
void term_screen_clear_and_home(void);
void term_screen_restore_full(void);

int terminal_check_size(int min_width, int min_height);

#endif
