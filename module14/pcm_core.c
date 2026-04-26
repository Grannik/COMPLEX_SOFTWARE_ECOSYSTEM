#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>

#include "../libtermcontrol/libtermcontrol.h"
#include "../libstrprepare/libstrprepare.h"
#include "../libtermcolor/libtermcolor.h"
#include "../common.h"
#include "pcm_core.h"

int show_error = 0;
char error_msg[256] = {0};
char *next_file_to_play = NULL;
char *next_file_name_to_play = NULL;

FILE *alsa_log_file = NULL;

const char *__lsan_default_options(void) {
 return "suppressions=asan.supp:print_suppressions=0";
}

void action_set_stop(PlayerControl *control, void *user_data) {
(void)user_data;
control->stop = 1;
}

void action_set_stop_playlist(PlayerControl *control, void *user_data) {
(void)user_data;
control->stop = 1;
control->playlist_mode = 0;
}

void with_mutex(PlayerControl *control, void (*action)(PlayerControl *, void *), void *user_data, int do_signal) {
    SAFE_MUTEX_LOCK(&control->mutex);
    if (action) action(control, user_data);
    if (do_signal) pthread_cond_signal(&control->cond);
    pthread_mutex_unlock(&control->mutex);
}

void load_playlist(const char *dir_path, PlayerControl *control) {
    with_mutex(control, action_set_stop_playlist, NULL, 1);
    show_error = 0;
    error_msg[0] = '\0';
    with_mutex(control, action_set_stop, NULL, 1);
struct LoadMainData data = {dir_path};
    with_mutex(control, action_load_main, &data, 0);
}

void shutdown_player_thread(PlayerControl *control, pthread_t thread, int *have_player_thread)
{
    if (!*have_player_thread) return;
    lock_and_signal(control, NULL);
    control->quit = 1;
    int joined = 0;
    time_t start = time(NULL);
    while (!joined && (time(NULL) - start < 1)) {
        int ret = pthread_kill(thread, 0);
        if (ret == ESRCH) {
            joined = 1;
        } else if (ret == 0) {
        }
    }
    if (joined) {
        pthread_join(thread, NULL);
    } else {
        pthread_detach(thread);
        fflush(stderr);
    }
    *have_player_thread = 0;
    control->quit = 0;
}

void alsa_log_handler(const char *file, int line, const char *function, int err, const char *fmt, ...) {
    if (alsa_log_file == NULL) {
        alsa_log_file = fopen("alsa_errors.log", "a");
    }
    if (alsa_log_file) {
        fprintf(alsa_log_file, "[ALSA Error] %s:%d (%s) err=%d: ", file, line, function, err);
        va_list arg;
        va_start(arg, fmt);
        vfprintf(alsa_log_file, fmt, arg);
        va_end(arg);
        fprintf(alsa_log_file, "\n");
        fflush(alsa_log_file);
    }
}

void display_message(int type, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *msg = (type == ERROR) ? error_msg : status_msg;
    size_t msg_size = sizeof(error_msg);
    vsnprintf(msg, msg_size, fmt, ap);
    if (type == ERROR) {
        show_error = 1;
    } else if (type == STATUS) {
        show_status = 1;
        status_start_time = time(NULL);
    }
    va_end(ap);
}

void play_audio(snd_pcm_t *handle, char *buffer, int size) {
    if (!handle || !buffer || size <= 0) {
       display_message(ERROR, "Invalid params in play_audio");
        return;
    }
    snd_pcm_uframes_t frames = (snd_pcm_uframes_t)(size / 4);
    char *ptr = buffer;
    snd_pcm_uframes_t remaining = frames;
    while (remaining > 0) {
        snd_pcm_sframes_t written = snd_pcm_writei(handle, ptr, remaining);
    if (written < 0) {
if (written == (snd_pcm_sframes_t)-EAGAIN) {
            continue;
        }
        if (written == -EPIPE) {
            if (snd_pcm_prepare(handle) < 0) {
                display_message(ERROR, "snd_pcm_prepare failed after EPIPE");
                snd_pcm_drop(handle);
                return;
            }
        } else {
            display_message(ERROR, "snd_pcm_writei failed: %s", snd_strerror((int)written));
            snd_pcm_drop(handle);
            return;
        }
        continue;
    }
        remaining -= (snd_pcm_uframes_t)written;
        ptr += written * 4;
    }
}

void memory_error(void) {display_message(ERROR, "Out of memory");}

void check_alloc(void *ptr) {if (!ptr) memory_error();}

char *safe_strdup(const char *src) {
    if (!src) return NULL;
    char *dup = strdup(src);
    check_alloc(dup);
    return dup;
}

FILE* open_audio_file(const char *filename) {
    if (!filename) {
       display_message(ERROR, "Invalid filename in open_audio_file");
        return NULL;
    }
int fd = open(filename, O_RDONLY | O_CLOEXEC);
if (fd == -1) {
    display_message(ERROR, "Failed to open file: %s", filename);
    return NULL;
}
FILE *file = fdopen(fd, "rb");
if (!file) {
    close(fd);
    display_message(ERROR, "fdopen failed for: %s", filename);
    return NULL;
}
    return file;
}

void safe_cleanup_resources(FILE **file, snd_pcm_t **handle, struct pollfd **poll_fds, char **current_filename) {
    if (file && *file) { fclose(*file); *file = NULL; }
    if (handle && *handle) {
if (*handle) {
    snd_pcm_drop(*handle);
    snd_pcm_close(*handle);
    *handle = NULL;
}
    }
    if (poll_fds && *poll_fds) { free(*poll_fds); *poll_fds = NULL; }
    if (current_filename && *current_filename) { free(*current_filename); *current_filename = NULL; }
}

void cleanup_playlist_and_filename(PlayerControl *control) {
    if (!control) return;
    SAFE_FREE(control->filename);
    SAFE_FREE(control->current_filename);
    if (control->playlist) {
        free_names(control->playlist, control->playlist_size, 0);
        control->playlist = NULL;
        control->playlist_size = 0;
        control->playlist_capacity = 0;
    }
    if (control->playlist_dir) {
        free(control->playlist_dir);
        control->playlist_dir = NULL;
    }
    control->current_track = 0;
    control->playlist_mode = 0;
}

void init_player_mutex(void) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&player_control.mutex, &attr);
        pthread_cond_init(&player_control.cond, NULL);
        pthread_mutexattr_destroy(&attr);
}

int setup_alsa_hw_params(
snd_pcm_t *handle, snd_pcm_hw_params_t *params, unsigned int *rate, int channels, snd_pcm_uframes_t *period_size, snd_pcm_uframes_t *buffer_size) {
    int dir = 0;
    int ret;
    ret = snd_pcm_hw_params_any(handle, params);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_any failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_access(
            handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_access failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_format(
            handle, params, SND_PCM_FORMAT_S16_LE);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_format failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_channels(handle, params, (unsigned int)channels);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_channels failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_rate_near(
            handle, params, rate, &dir);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_rate_near failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_period_size_near(
            handle, params, period_size, &dir);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_period_size_near failed", 1) < 0)
        return -1;
    ret = snd_pcm_hw_params_set_buffer_size_near(
            handle, params, buffer_size);
    if (handle_alsa_error(ret, "snd_pcm_hw_params_set_buffer_size_near failed", 1) < 0)
        return -1;
    if (handle_alsa_error(
            snd_pcm_hw_params(handle, params),
            "snd_pcm_hw_params failed", 1) < 0) {
        return -1;
    }
    return 0;
}

int handle_alsa_error(int ret, const char *msg, int do_return) {
    if (ret < 0) {
        const char *err = snd_strerror(ret);
        if (err) {
            display_message(ERROR, "%s: %s — audio disabled", msg, err);
        } else {
            display_message(ERROR, "%s — audio disabled", msg);
        }
        if (do_return) return -1;
    }
    return 0;
}

snd_pcm_t* init_audio_device(unsigned int rate, int channels, int nonblock) {
    snd_pcm_t *handle = NULL;
    int mode = nonblock ? SND_PCM_NONBLOCK : 0;
    int ret = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, mode);
    if (handle_alsa_error(ret, "ALSA device open error", 1) < 0) {
        return NULL;
    }
    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_uframes_t period_size = 1024;
    snd_pcm_uframes_t buffer_size = 4096;
    if (setup_alsa_hw_params(handle, params, &rate, channels, &period_size, &buffer_size) < 0) {
        snd_pcm_close(handle);
        return NULL;
    }

    ret = snd_pcm_prepare(handle);
    if (handle_alsa_error(ret, "snd_pcm_prepare failed", 1) < 0) {
        snd_pcm_close(handle);
        return NULL;
    }
    return handle;
}

void cleanup_playlist(PlayerControl *control) {
    if (!control) return;
    char **old_list = control->playlist;
    int old_size = control->playlist_size;
    char *old_dir = control->playlist_dir;
    control->playlist = NULL;
    control->playlist_size = 0;
    control->playlist_capacity = 0;
    control->current_track = 0;
    control->playlist_dir = NULL;
    if (old_list) {
        free_names(old_list, old_size, 0);
    }
    if (old_dir) {
        free(old_dir);
    }
}

void action_s(PlayerControl *control) {
    if (control->current_file && control->current_filename && !control->paused) {
        control->fading_out = 1;
        control->current_fade = FADE_STEPS;
        control->is_silent = 0;
        control->stop = 0;
        display_message(STATUS, "Fading before stopping...");
    } else {
        control->stop = 1;
        control->duration = 0.0;
        control->bytes_read = 0LL;
        show_error = 0;
        error_msg[0] = '\0';
        if (control->current_filename) {
            free(control->current_filename);
            control->current_filename = NULL;
        }
        control->paused = 0;
        control->fading_out = 0;
        cleanup_playlist(control);
        control->playlist_mode = 0;
        control->current_track = 0;
        display_message(STATUS, "Playback stopped");
    }
}

void fade_in_on_start(void) {
    SAFE_MUTEX_LOCK(&player_control.mutex);
    player_control.fading_out = 0;
    player_control.fading_in = 0;
    player_control.current_fade = 0;
    player_control.is_silent = 0;
    pthread_mutex_unlock(&player_control.mutex);
    SAFE_MUTEX_LOCK(&player_control.mutex);
    player_control.fading_in = 1;
    player_control.current_fade = 0;
    pthread_cond_signal(&player_control.cond);
    pthread_mutex_unlock(&player_control.mutex);
}

 void play_single_file(void) {
    if (!next_file_to_play || !next_file_name_to_play) {
        display_message(ERROR, "No file to play");
        return;
    }
    SAFE_MUTEX_LOCK(&player_control.mutex);
    SAFE_FREE(player_control.filename);
    player_control.filename = strdup(next_file_to_play);
    SAFE_FREE(player_control.current_filename);
    player_control.current_filename = strdup(next_file_name_to_play);
    player_control.stop = 0;
    player_control.paused = 0;
    pthread_cond_signal(&player_control.cond);
    pthread_mutex_unlock(&player_control.mutex);
    SAFE_FREE(next_file_to_play);
    SAFE_FREE(next_file_name_to_play);
}

void apply_fade(PlayerControl *ctrl, int fade_dir, char *buffer, int size) {
    int16_t *samples = (int16_t *)buffer;
    size_t num_samples = (size_t)(size / 2);
    float factor_start = (float)ctrl->current_fade / FADE_STEPS;
    float factor_end;
    if (fade_dir < 0) {
        factor_end = (float)(ctrl->current_fade - 1) / FADE_STEPS;
        if (factor_end < 0) factor_end = 0.0f;
    } else {
        factor_end = (float)(ctrl->current_fade + 1) / FADE_STEPS;
    }
    for (size_t i = 0; i < num_samples; i++) {
        float interp = (num_samples > 1) ? (float)i / (float)(num_samples - 1) : 0.0f;
        float factor = factor_start + (factor_end - factor_start) * interp;
        samples[i] = (int16_t)(samples[i] * factor);
    }
    ctrl->current_fade += fade_dir;
    if (fade_dir < 0 && ctrl->current_fade <= 0) {
        ctrl->fading_out = 0;
        ctrl->current_fade = 0;
        ctrl->is_silent = 1;
        if (!ctrl->paused) {
            ctrl->stop = 1;
        display_message(STATUS, "Fade completed, stopping");
        }
    } else if (fade_dir > 0 && ctrl->current_fade >= FADE_STEPS) {
        ctrl->fading_in = 0;
        ctrl->current_fade = FADE_STEPS;
        ctrl->is_silent = 0;
    }
}

void action_p(PlayerControl *control) {
    if (!control->current_file) {
        display_message(STATUS, "Nothing to pause");
        return;
    }
    if (control->paused) {
        control->paused = 0;
        control->is_silent = 0;
        control->fading_in = 1;
        control->current_fade = 0;
        control->fading_out = 0;
        display_message(STATUS, "RESUMED (smooth fade-in)");
    } else {
        control->paused = 1;
        control->fading_out = 1;
        control->current_fade = FADE_STEPS;
        display_message(STATUS, "PAUSED (smooth fade-out)");
    }
}

void lock_and_signal(PlayerControl *control, void (*action)(PlayerControl *)) {
    pthread_mutex_lock(&control->mutex);
    if (action) action(control);
    pthread_cond_signal(&control->cond);
    pthread_mutex_unlock(&control->mutex);
}

int load_raw_files(const char *dir_path, char ***files_out, int *count_out) {
    void *entries = NULL;
    if (scan_directory_ex(dir_path, FILE_CATEGORY_PCM | FILE_CATEGORY_RAW, OUTPUT_MODE_PLAYLIST, (void **)&entries, count_out) != 0) {
    return -1;
}
    if (*count_out == 0) {
        free(entries);
        return 0;
    }
    if (entries && *count_out > 0) {
        setlocale(LC_COLLATE, "");
        qsort(entries, (size_t)*count_out, sizeof(char *), playlist_cmp);
    }
    *files_out = (char **)entries;
    return 0;
}

void assign_safe_strdup(char **dest, const char *src) {
     SAFE_FREE(*dest);
     *dest = safe_strdup(src);
}

void action_load_main(PlayerControl *control, void *user_data) {
    struct LoadMainData *d = user_data;
    while (control->stop == 1) {
        struct timespec ts_wait;
        clock_gettime(CLOCK_REALTIME, &ts_wait);
        ts_wait.tv_sec += 1;
        int ret = pthread_cond_timedwait(&control->cond, &control->mutex, &ts_wait);
        if (ret == ETIMEDOUT) {
            display_message(ERROR, "Warning: audio stream not responding — continuing without waiting");
            break;
        }
    }
    if (control->playlist) {
        free_names(control->playlist, control->playlist_size, 0);
    }
    control->playlist = NULL;
    control->playlist_size = 0;
    control->playlist_capacity = 0;
    control->current_track = 0;
    control->playlist_dir = NULL;
    char **entries = NULL;
    int count = 0;
    if (load_raw_files(d->dir_path, &entries, &count) != 0 || count == 0) {
        if (entries) free(entries);
        control->playlist_mode = 0;
        return;
    }
    control->playlist = entries;
    control->playlist_capacity = count;
    control->playlist_size = count;
    control->current_track = 0;
    control->playlist_mode = 1;
    if (control->playlist_size > 0 && control->playlist[0]) {
        if (control->filename) free(control->filename);
        SAFE_FREE(control->filename);
        control->filename = strdup(control->playlist[0]);
        pthread_cond_signal(&control->cond);
        assign_safe_strdup(&control->playlist_dir, d->dir_path);
    }
}

void perform_seek(PlayerControl *control, snd_pcm_t *handle)
{
    if (!control->current_file || control->seek_delta == 0) {
        control->seek_delta = 0;
        return;
    }
    long long seek_bytes = (long long)control->seek_delta * BYTES_PER_SECOND;
    pthread_mutex_lock(&control->mutex);
    long current_pos = ftell(control->current_file);
    pthread_mutex_unlock(&control->mutex);
    long long new_pos = current_pos + seek_bytes;
    long file_size = 0;
    pthread_mutex_lock(&control->mutex);
    long original_pos = ftell(control->current_file);
    if (fseek(control->current_file, 0, SEEK_END) == 0) {
        file_size = ftell(control->current_file);
        fseek(control->current_file, original_pos, SEEK_SET);
    }
    pthread_mutex_unlock(&control->mutex);
    if (new_pos < 0) new_pos = 0;
    if (new_pos > file_size) new_pos = file_size;
    new_pos = (new_pos / 4) * 4;
    if (new_pos == current_pos) {
        control->seek_delta = 0;
        return;
    }
    if (handle && !control->is_silent) {
        control->fading_out = 1;
        control->current_fade = FADE_STEPS;
    }
    pthread_mutex_lock(&control->mutex);
    fseek(control->current_file, new_pos, SEEK_SET);
    control->bytes_read = new_pos;
    if (handle) {
        snd_pcm_drop(handle);
        snd_pcm_prepare(handle);
        char silence_buffer[8192];
        memset(silence_buffer, 0, sizeof(silence_buffer));
        for (int i = 0; i < 3; i++) {
            snd_pcm_writei(handle, silence_buffer, sizeof(silence_buffer) / FRAME_SIZE);
        }
    }
    control->fading_out = 0;
    control->fading_in = 1;
    control->current_fade = 0;
    control->is_silent = 0;
    control->seek_delta = 0;
    pthread_mutex_unlock(&control->mutex);
}

void *player_thread(void *arg) {
    PlayerControl *control = (PlayerControl *)arg;
    unsigned int rate = 44100;
    int channels = 2;
    snd_pcm_t *handle = NULL;
    FILE *file = NULL;
    int local_paused = 0;
    const int buffer_size = 4096;
    char buffer[buffer_size];
    unsigned int poll_count = 0;
    struct pollfd *poll_fds = NULL;
    while (1) {
        pthread_mutex_lock(&control->mutex);
        if (control->quit) {
            pthread_mutex_unlock(&control->mutex);
            break;
        }
if (control->stop) {
        safe_cleanup_resources(&file, &handle, &poll_fds, &control->current_filename);
        cleanup_playlist_and_filename(control);
        if (control->playlist_mode) {
 display_message(STATUS, "Playlist completed");
        }
        control->stop = 0;
        control->duration = 0.0;
        control->bytes_read = 0LL;
        control->paused = 0;
        control->playlist_mode = 0;
        control->current_track = 0;
        pthread_cond_signal(&control->cond);
        pthread_mutex_unlock(&control->mutex);
        continue;
    }
        if (control->filename && (!control->current_filename || strcmp(control->filename, control->current_filename) != 0)) {
           safe_cleanup_resources(&file, &handle, NULL, &control->current_filename);
           free(poll_fds); poll_fds = NULL;
           file = open_audio_file(control->filename);
    if (file) {
        struct stat st;
        if (fstat(fileno(file), &st) == 0) {
            long file_size = st.st_size;
            control->duration = (double)file_size / BYTES_PER_SECOND;
            control->bytes_read = 0LL;
        } else {
            control->duration = 0.0;
            control->bytes_read = 0LL;
        }

        handle = init_audio_device(rate, channels, 1);
        if (!handle) {
            safe_cleanup_resources(&file, &handle, &poll_fds, NULL);
            pthread_mutex_unlock(&control->mutex);
            continue;
        }
        control->current_file = file;
        control->current_filename = (control->filename) ? SAFE_STRDUP(control->filename) : NULL;
if (control->current_filename) {
long original_pos = ftell(file);
if (fseek(file, 0, SEEK_END) == 0) {
    long file_size = ftell(file);
    fseek(file, original_pos, SEEK_SET);
    control->duration = (double)file_size / BYTES_PER_SECOND;
} else {
    control->duration = 0.0;
}
                            control->is_silent = 0;
                            control->fading_in = 0;
                            control->fading_out = 0;
                            control->current_fade = FADE_STEPS;
                            poll_count = (unsigned int)snd_pcm_poll_descriptors_count(handle);
 if (poll_count > 0) {
     poll_fds = malloc(poll_count * sizeof(struct pollfd));
     if (poll_fds) {
         snd_pcm_poll_descriptors(handle, poll_fds, poll_count);
     } else {
safe_cleanup_resources(&file, &handle, NULL, NULL);
         continue;
     }
 }
                                char silence_buffer[16384];
                                memset(silence_buffer, 0, sizeof(silence_buffer));
                                snd_pcm_sframes_t frames_to_write = sizeof(silence_buffer) / FRAME_SIZE;
                                snd_pcm_sframes_t written = 0;
                                while (written < frames_to_write) {
                                snd_pcm_sframes_t w = snd_pcm_writei(handle,
                                     silence_buffer + written * FRAME_SIZE,
                                     (snd_pcm_uframes_t)(frames_to_write - written));
if (w < 0) {
snd_pcm_recover(handle, (int)w, 1);
    continue;
}
                                    written += w;
                                }
                } else {
SAFE_FREE(control->filename);
                }
            } else {
                free(control->filename);
                control->filename = NULL;
            }
        }
        pthread_mutex_unlock(&control->mutex);

                if (handle && file && !local_paused) {
                    if (!poll_fds || poll_count <= 0) {
                        continue;
                    }
                    if (poll(poll_fds, poll_count, 100) < 0) continue;

            unsigned short revents;
            snd_pcm_poll_descriptors_revents(handle, poll_fds, poll_count, &revents);
            if (revents & POLLOUT) {
SAFE_MUTEX_LOCK(&control->mutex);
perform_seek(control, handle);
    if (control->seek_delta != 0) {
        continue;
    }
pthread_mutex_unlock(&control->mutex);
size_t read_size;
                pthread_mutex_lock(&control->mutex);
                if (control->is_silent) {
                    memset(buffer, 0, (size_t)buffer_size);
                    read_size = (size_t)buffer_size;
                } else {
                    read_size = fread(buffer, 1, (size_t)buffer_size, file);
                    if (read_size % 4 != 0) {
                        read_size -= read_size % 4;
                        if (ferror(file)) {
                            break;
                        }
                    }
        if (read_size == 0) {
    if (feof(file)) {
        if (control->playlist_mode && control->current_track < control->playlist_size - 1) {
            safe_cleanup_resources(&file, &handle, &poll_fds, &control->current_filename);
            control->current_track++;
            SAFE_FREE(control->filename);
            control->filename = SAFE_STRDUP(control->playlist[control->current_track]);
            if (!control->filename) {
                control->current_track = control->playlist_size;
            }
            pthread_mutex_unlock(&control->mutex);
            continue;
        } else if (control->loop_mode && !control->playlist_mode) {
            fseek(file, 0, SEEK_SET);
            control->bytes_read = 0LL;
            control->seek_delta = 0;
            pthread_mutex_unlock(&control->mutex);
            continue;
        } else {
            if (control->playlist_mode) {
                display_message(STATUS, "End of playlist reached");
            }
            if (control->playlist) {
                free_names(control->playlist, control->playlist_size, 0);
                control->playlist = NULL;
                control->playlist_size = 0;
                control->playlist_capacity = 0;
                free(control->playlist_dir);
                control->playlist_dir = NULL;
            }
            SAFE_FREE(control->filename);
            safe_cleanup_resources(&file, &handle, &poll_fds, &control->current_filename);
            control->playlist_mode = 0;
            control->duration = 0.0;
            control->bytes_read = 0LL;
            pthread_mutex_unlock(&control->mutex);
            continue;
        }
    } else {
        display_message(ERROR, "File read error");
        if (control->playlist) {
            free_names(control->playlist, control->playlist_size, 0);
            control->playlist = NULL;
            control->playlist_size = 0;
            control->playlist_capacity = 0;
            free(control->playlist_dir);
            control->playlist_dir = NULL;
        }
        if (control->filename) {
            free(control->filename);
            control->filename = NULL;
        }
        safe_cleanup_resources(&file, &handle, &poll_fds, &control->current_filename);
        cleanup_playlist_and_filename(control);
        control->playlist_mode = 0;
        control->duration = 0.0;
        control->bytes_read = 0LL;
        pthread_mutex_unlock(&control->mutex);
        continue;
    }
}
        control->bytes_read += (long long)read_size;
    }
    pthread_mutex_unlock(&control->mutex);
    int actual_size = (int)((read_size / 4) * 4);
    if (actual_size <= 0) continue;
    pthread_mutex_lock(&control->mutex);
    if (control->fading_out || control->fading_in) {
        int dir = control->fading_out ? -1 : 1;
        apply_fade(control, dir, buffer, actual_size);
    }
    pthread_mutex_unlock(&control->mutex);
    play_audio(handle, buffer, actual_size);
            }
        } else {
        }
    }
    safe_cleanup_resources(&file, &handle, &poll_fds, &control->current_filename);
    cleanup_playlist_and_filename(control);
    return NULL;
}

void print_formatted_time(WINDOW *win, int y, int x, int seconds) {
    if (seconds < 0) {
        mvwprintw(win, y, x, "--:--:--");
        return;
    }
    int hours = seconds / 3600;
    int mins = (seconds % 3600) / 60;
    int secs = seconds % 60;
    mvwprintw(win, y, x, "%02d:%02d:%02d", hours, mins, secs);
}

void free_file_list(void) {
    if (file_list) {
        free_entries((void**)&file_list, &file_cnt, OUTPUT_MODE_FILE_LIST);
    }
    file_list = NULL;
    file_cnt = 0;
    selected_index = -1;
}

void update_filelist(void) {
    free_file_list();
int count = 0;
FileEntry *entries = NULL;
if (scan_directory_ex(curr_dir, 0, OUTPUT_MODE_FILE_LIST,
                     (void **)&entries, &count) != 0) {
    file_list = calloc(1, sizeof(FileEntry));
    file_list[0].name = strdup("(access denied)");
    file_list[0].is_dir = 0;
    file_cnt = 1;
    selected_index = 0;
    frame_control(TITLE_WARNING, MSG_WARNING, "Access denied to: %s", curr_dir);
    return;
} if (count == 0) {
    file_list = calloc(1, sizeof(FileEntry));
    if (file_list) {
        file_list[0].name = strdup("Directory is empty or not enough memory.");
        file_list[0].is_dir = 0;
        file_cnt = 1;
    }
    selected_index = 0;
    free(entries);
    return;
}
qsort(entries, (size_t)count, sizeof(FileEntry), file_entry_cmp);
file_list = entries;
file_cnt = count;
selected_index = 0;
}

void draw_progress_bar(WINDOW *win, int y, int start_x, double percent, int bar_length) {
    if (percent < 0.0 || isnan(percent)) percent = 0.0;
    if (percent > 100.0) percent = 100.0;
    int filled = (int)((percent / 100.0) * bar_length + 0.5);
    if (filled > bar_length) filled = bar_length;
    wchar_t fill_char = SCROLL_FILLED;
    wchar_t empty_char = SCROLL_EMPTY;
    int color_empty = COLOR_PAIR_WHITE;
    if (percent > 0.0) color_empty = COLOR_PURPLE;
        wattron(win, COLOR_PAIR(color_empty));
        COLOR_ATTR_ON(win, color_empty);
        draw_fill_line(win, y, start_x, bar_length, &empty_char, 1, 1);
    if (filled > 0) {
        COLOR_ATTR_ON(win, COLOR_PURPLE);
        draw_fill_line(win, y, start_x, filled, &fill_char, 1, 1);
        COLOR_ATTR_OFF(win, COLOR_PURPLE);
}
}

void action_next_prev(PlayerControl *control, int direction) {
    (void)control;
    if (file_cnt == 0) return;
    const char *current_full_path = NULL;
    const char *current_playing = NULL;
    SAFE_MUTEX_LOCK(&player_control.mutex);
    if (player_control.current_filename) {
        current_full_path = player_control.current_filename;
        const char *slash = strrchr(current_full_path, '/');
        current_playing = slash ? slash + 1 : current_full_path;
    }
    pthread_mutex_unlock(&player_control.mutex);
    if (!current_playing) {
        display_message(ERROR, "Nothing is playing");
        return;
    }
    int playing_idx = -1;
    for (int i = 0; i < file_cnt; i++) {
        if (file_list[i].name && !file_list[i].is_dir &&
            strcmp(file_list[i].name, current_playing) == 0) {
            playing_idx = i;
            break;
        }
    }
    if (playing_idx == -1) {
        display_message(ERROR, "Current file '%s' not found in list", current_playing);
        return;
    }
    int next_idx = -1;
    if (direction == 1) {
        for (int i = playing_idx + 1; i < file_cnt; i++) {
    if (!file_list[i].is_dir && is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM)) {
                next_idx = i;
                break;
            }
        }
        if (next_idx == -1) {
            for (int i = 0; i < file_cnt; i++) {
    if (!file_list[i].is_dir && is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM)) {
                    next_idx = i;
                    break;
                }
            }
        }
    } else {
        for (int i = playing_idx - 1; i >= 0; i--) {
    if (!file_list[i].is_dir && is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM)) {
                next_idx = i;
                break;
            }
        }
        if (next_idx == -1) {
            for (int i = file_cnt - 1; i >= 0; i--) {
        if (!file_list[i].is_dir && is_file_type(file_list[i].name, FILE_TYPE_AUDIO_PCM)) {
                    next_idx = i;
                    break;
                }
            }
        }
    }
    if (next_idx == -1) {
        display_message(ERROR, "No more .raw files");
        return;
    }
    lock_and_signal(&player_control, action_s);
    char *full_path = xasprintf("%s/%s", curr_dir, file_list[next_idx].name);
    if (!full_path) {
        display_message(ERROR, "Out of memory");
        return;
    }
    SAFE_FREE(next_file_to_play);
    SAFE_FREE(next_file_name_to_play);
    next_file_to_play = strdup(full_path);
    next_file_name_to_play = strdup(file_list[next_idx].name);
    free(full_path);
    if (!next_file_to_play || !next_file_name_to_play) {
        SAFE_FREE(next_file_to_play);
        SAFE_FREE(next_file_name_to_play);
        display_message(ERROR, "Out of memory");
        return;
    }
    fade_in_on_start();
    play_single_file();
    display_message(STATUS, "%s: %s", direction == 1 ? "Next" : "Previous", file_list[next_idx].name);
}

int try_start_player_thread(pthread_t *thread, void *(*func)(void *), void *arg)
{
if (pthread_create(thread, NULL, func, arg) == 0) {
    return 1;
}
fprintf(stderr, "pthread_create failed — player disabled\n");
    return 0;
}

void handle_program_exit(int result, int was_playing, int hours, int mins, int secs) {
    if (result == 0) {
        terminal_control(0);
        if (was_playing) {
            printf(
                "\033[31mWARNING: Audio thread did not finish in 3s — "
                "interrupted at %02d:%02d:%02d — "
                "leaving it (safer than pthread_cancel)\033[0m\n",
                hours, mins, secs
            );
      }
    }
  }

void start_playback(const char *full_path, const char *file_name, int enable_loop) {
    if (!full_path || !file_name) return;
    show_error = 0;
    error_msg[0] = '\0';
    struct stat st;
    if (stat(full_path, &st) == 0) {
        if (st.st_size == 0) {
            display_message(ERROR, "File '%s' is empty (0 bytes)", file_name);
            return;
        }
        if (st.st_size < 4) {
            display_message(ERROR, "File '%s' too small for a single frame (%ld bytes < 4)", file_name, st.st_size);
            return;
        }
        if (st.st_size % 4 != 0) {
            display_message(ERROR,
                "File '%s': size %ld bytes not multiple of 4. "
                "Required: stereo 16-bit RAW (2ch × 2bytes = 4bytes/frame)",
                file_name, st.st_size);
            return;
        }
        if (st.st_size < 1024) {
            display_message(STATUS,
                "File '%s' is very small (%ld bytes). Playback may be short.",
                file_name, st.st_size);
        }
    } else {
        display_message(ERROR, "Cannot access file: %s", file_name);
        return;
    }
    FILE *temp_file = fopen(full_path, "rb");
    if (!temp_file) {
        display_message(ERROR, "Failed to open for validation: %s", file_name);
        return;
    }
    char header[4] = {0};
    size_t read = fread(header, 1, 4, temp_file);
    fclose(temp_file);
    if (read >= 4) {
        if (memcmp(header, "RIFF", 4) == 0 || memcmp(header, "OggS", 4) == 0 ||
            memcmp(header, "fLaC", 4) == 0 || memcmp(header, "FORM", 4) == 0) {
            display_message(ERROR, "File '%s' appears to be formatted audio (e.g., WAV/OGG/FLAC/AIFF), not raw PCM", file_name);
            return;
        }
    }
    SAFE_FREE(next_file_to_play);
    SAFE_FREE(next_file_name_to_play);
    next_file_to_play = strdup(full_path);
    next_file_name_to_play = strdup(file_name);
    if (!next_file_to_play || !next_file_name_to_play) {
        SAFE_FREE(next_file_to_play);
        SAFE_FREE(next_file_name_to_play);
        display_message(ERROR, "Out of memory! Cannot play file.");
        return;
    }
    pthread_mutex_lock(&player_control.mutex);
    cleanup_playlist(&player_control);
    player_control.loop_mode = enable_loop;
    player_control.paused = 0;
    player_control.is_silent = 0;
    player_control.fading_in = 0;
    player_control.fading_out = 0;
    player_control.current_fade = FADE_STEPS;
    player_control.bytes_read = 0LL;
    player_control.duration = 0.0;
    player_control.seek_delta = 0;
    pthread_cond_signal(&player_control.cond);
    pthread_mutex_unlock(&player_control.mutex);
    play_single_file();
    fade_in_on_start();
    if (enable_loop) {
        display_message(STATUS, "Playback started in loop mode on new file");
    } else {
        display_message(STATUS, "Playback started (smooth fade-in)");
    }
}

void lock_and_signal_seek(PlayerControl *control, int delta, const char *msg) {
    SAFE_MUTEX_LOCK(&player_control.mutex);
    action_seek(control, delta, msg);
    pthread_cond_signal(&control->cond);
    pthread_mutex_unlock(&control->mutex);
}

void action_seek(PlayerControl *control, int delta, const char *msg_if_none) {
    if (control->current_file) {
        control->seek_delta = delta;
    } else {
        display_message(STATUS, "%s", msg_if_none);
    }
}

int keys_play(int ch) {
    switch (ch) {
case '=':
case '-': {
    int delta = (ch == '=') ? 10 : -10;
    const char *msg = (ch == '=') ? "Nothing to fast-forward" : "Nothing to rewind";
    lock_and_signal_seek(&player_control, delta, msg);
    break;
}
case ' ':
    if (file_cnt > 0 && selected_index >= 0 && file_list && file_list[selected_index].name && file_list[selected_index].is_dir) {
        char *full_path = xasprintf("%s/%s", curr_dir, file_list[selected_index].name);
        if (!full_path) {
            display_message(STATUS, "Out of memory!");
        } else {
            load_playlist(full_path, &player_control);
            free(full_path);
            if (player_control.playlist_size == 0) {
                display_message(ERROR, "Directory does not contain raw files.");
            } else {
                display_message(STATUS, "Playlist loaded from %s", file_list[selected_index].name);
            }
        }
    } else {
        load_playlist(curr_dir, &player_control);
        if (player_control.playlist_size == 0) {
            display_message(ERROR, "No .raw files in current directory.");
        } else {
            display_message(STATUS, "Playlist loaded from current directory");
        }
    }
    break;
        case '\\':
        case '/':
        {
            int dir = (ch == '\\') ? 1 : -1;
            SAFE_MUTEX_LOCK(&player_control.mutex);
            action_next_prev(&player_control, dir);
            pthread_mutex_unlock(&player_control.mutex);
}
            break;
        case ';':
        {
            int different = 0;
            const char *selected_name = NULL;
            pthread_mutex_lock(&player_control.mutex);
            if (file_cnt > 0 && selected_index >= 0 && file_list && file_list[selected_index].name &&
            !file_list[selected_index].is_dir && is_file_type(file_list[selected_index].name, FILE_TYPE_AUDIO_PCM)) {
                selected_name = file_list[selected_index].name;
                different = selected_name &&
                    (!player_control.current_filename || strcmp(player_control.current_filename, selected_name) != 0);
            }
            pthread_mutex_unlock(&player_control.mutex);
            if (different && selected_name) {
                char *full_path = xasprintf("%s/%s", curr_dir, selected_name);
                if (full_path) {
                    start_playback(full_path, selected_name, 1);
                    free(full_path);
                } else {
                    display_message(STATUS, "Out of memory! Cannot play file.");
                }
            } else {
                pthread_mutex_lock(&player_control.mutex);
                if (player_control.current_file || player_control.filename) {
                    player_control.loop_mode = !player_control.loop_mode;
                    display_message(STATUS, "%s", player_control.loop_mode ? "Loop mode enabled" : "Loop mode disabled");
                } else {
                    display_message(STATUS, "Nothing to loop");
                }
                pthread_mutex_unlock(&player_control.mutex);
            }
            break;
        }
case '.':
lock_and_signal(&player_control, action_s);
break;
        default:
            return 0;
    }
    return 1;
}

int run_player_system(int (*nav_func)(void)) {
    init_player_mutex();
    pthread_t thread = 0;
    int have_player_thread = 0;
    have_player_thread = try_start_player_thread(&thread, player_thread, &player_control);
    int result = nav_func();
    SAFE_MUTEX_LOCK(&player_control.mutex);
    bool was_playing = (player_control.current_filename != NULL);
    double elapsed = 0.0;
    if (was_playing) {
        elapsed = (double)player_control.bytes_read / BYTES_PER_SECOND;
    }
    pthread_mutex_unlock(&player_control.mutex);
    int hours = (int)elapsed / 3600;
    int mins = ((int)elapsed % 3600) / 60;
    int secs = (int)elapsed % 60;
    if (have_player_thread) {
        shutdown_player_thread(&player_control, thread, &have_player_thread);
    }
    lock_and_signal(&player_control, cleanup_playlist_and_filename);
    if (have_player_thread) {
        pthread_mutex_destroy(&player_control.mutex);
        pthread_cond_destroy(&player_control.cond);
    }
    handle_program_exit(result, was_playing, hours, mins, secs);
    snd_config_update_free_global();
    return result;
}
