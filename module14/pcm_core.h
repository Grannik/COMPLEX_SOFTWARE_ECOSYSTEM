#ifndef PCM_PLAYER_H
#define PCM_PLAYER_H

#define STATUS 2
#define CHANNELS 2
#define FRAME_SIZE (CHANNELS * 2)
#define BYTES_PER_SECOND 176400LL
#define SAFE_STRDUP(src) safe_strdup(src)
#define SAFE_CLEANUP_RESOURCES(filep, handlep, pollfdsp, currfilep) safe_cleanup_resources((filep), (handlep), (pollfdsp), (currfilep))

#define COLOR_PAIR_PROGRESS get_color_pair(2, -1)
#define COLOR_PAIR_WHITE get_color_pair(7, -1)
#define COLOR_PAIR_BLUE get_color_pair(4, -1)
#define COLOR_PAIR_BORDER get_color_pair(2, -1)

#define COLOR_PAIR_RED get_color_pair(1, -1)
#define COLOR_PAIR_FATAL COLOR_PAIR_RED
#define COLOR_PAIR_WARNING_NORM COLOR_PAIR_RED

#define COLOR_PAIR_GRAY get_color_pair(8, -1)
#define COLOR_PAIR_DEBUG COLOR_PAIR_GRAY

#define COLOR_PAIR_YELLOW get_color_pair(3, -1)
#define COLOR_PAIR_NOTIFICATION COLOR_PAIR_YELLOW

#define COLOR_PAIR_CYAN get_color_pair(6, -1)
#define COLOR_PAIR_INFO COLOR_PAIR_CYAN

#define COLOR_ATTR_ON(win, attr) wattron(win, COLOR_PAIR(attr))
#define COLOR_ATTR_OFF(win, attr) wattroff(win, COLOR_PAIR(attr))

#define COLOR_PURPLE get_color_pair(5, -1)

extern int show_error;
extern int show_status;
extern char status_msg[256];
extern char error_msg[256];
extern char *next_file_to_play;
extern char *next_file_name_to_play;
extern time_t status_start_time;
extern FILE *alsa_log_file;

struct LoadMainData {
    const char *dir_path;
};

int setup_alsa_hw_params(
snd_pcm_t *handle, snd_pcm_hw_params_t *params, unsigned int *rate, int channels, snd_pcm_uframes_t *period_size, snd_pcm_uframes_t *buffer_size);
int handle_alsa_error(int ret, const char *msg, int do_return);
int load_raw_files(const char *dir_path, char ***files_out, int *count_out);
void alsa_log_handler(const char *file, int line, const char *function, int err, const char *fmt, ...);
void display_message(int type, const char *fmt, ...);
void play_audio(snd_pcm_t *handle, char *buffer, int size);
void memory_error(void);
void check_alloc(void *ptr);
void safe_cleanup_resources(FILE **file, snd_pcm_t **handle, struct pollfd **poll_fds, char **current_filename);
void cleanup_playlist_and_filename(PlayerControl *control);
void init_player_mutex(void);
void cleanup_playlist(PlayerControl *control);
void action_s(PlayerControl *control);
void fade_in_on_start(void);
char *safe_strdup(const char *src);
const char *__lsan_default_options(void);
FILE* open_audio_file(const char *filename);
snd_pcm_t* init_audio_device(unsigned int rate, int channels, int nonblock);
void play_single_file(void);
void apply_fade(PlayerControl *ctrl, int fade_dir, char *buffer, int size);
void action_p(PlayerControl *control);
void lock_and_signal(PlayerControl *control, void (*action)(PlayerControl *));
void *player_thread(void *arg);
void perform_seek(PlayerControl *control, snd_pcm_t *handle);
void action_load_main(PlayerControl *control, void *user_data);
void assign_safe_strdup(char **dest, const char *src);
void print_formatted_time(WINDOW *win, int y, int x, int seconds);
void free_file_list(void);
void update_filelist(void);
void draw_progress_bar(WINDOW *win, int y, int start_x, double percent, int bar_length);
void action_set_stop(PlayerControl *control, void *user_data);
void action_set_stop_playlist(PlayerControl *control, void *user_data);
void with_mutex(PlayerControl *control, void (*action)(PlayerControl *, void *), void *user_data, int do_signal);
void load_playlist(const char *dir_path, PlayerControl *control);
void shutdown_player_thread(PlayerControl *control, pthread_t thread, int *have_player_thread);
void action_next_prev(PlayerControl *control, int direction);
int try_start_player_thread(pthread_t *thread, void *(*func)(void *), void *arg);
void handle_program_exit(int result, int was_playing, int hours, int mins, int secs);
void start_playback(const char *full_path, const char *file_name, int enable_loop);
void lock_and_signal_seek(PlayerControl *control, int delta, const char *msg);
void action_seek(PlayerControl *control, int delta, const char *msg_if_none);
int keys_play(int ch);
int run_player_system(int (*nav_func)(void));

#endif
