#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "libtermanimation.h"

#define MAX_ANIM 32

#define MATRIX_WIDTH       50
#define MATRIX_HEIGHT      20
#define START_PROB          5
#define ADVANCE_HEAD_PROB  90
#define ADVANCE_TAIL_PROB  30
#define FLICKER_PROB        5

void clear_screen(void);
long time_ms(void);
typedef void (*frame_func)(void *data);
int anim_add(frame_func frame, void *data, int fps);
void anim_loop(void);
void spinner_frame(void *data);
void progress_frame(void *data);
void progress_frame(void *data);
int anim_add_progress(int x, int y, int width, int progress, int fps);
int anim_add_spinner(int x, int y, int fps);
void marquee_frame(void *data);
int anim_add_marquee(int x, int y, int width, const char *text, int fps);
void typewriter_frame(void *data);
int anim_add_typewriter(int x, int y, const char *text, int fps);
int anim_add_matrix_rain(int fps);
int anim_add_graph(int x, int y, int width, int height, int fps);
int anim_add_clock(int x, int y, int fps);
int anim_add_spectrum(int x, int y, int num_bars, int max_height, int fps);

void clear_screen(void){printf("\033[2J\033[H");}

typedef void (*frame_func)(void *data);

typedef struct
{
    frame_func frame;
    void *data;
    int fps;
    long next_time;
    int active;
} Animation;

Animation anims[MAX_ANIM];

long time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int anim_add(frame_func frame, void *data, int fps)
{
    for (int i = 0; i < MAX_ANIM; i++)
    {
        if (!anims[i].active)
        {
            anims[i].frame = frame;
            anims[i].data = data;
            anims[i].fps = fps;
            anims[i].next_time = time_ms();
            anims[i].active = 1;
            return i;
        }
    }
    return -1;
}

void anim_loop(void)
{
    while (1)
    {
        long now = time_ms();
        for (int i = 0; i < MAX_ANIM; i++)
        {
            if (!anims[i].active) continue;
            if (now >= anims[i].next_time)
            {
                anims[i].frame(anims[i].data);
                int delay = 1000 / anims[i].fps;
                anims[i].next_time = now + delay;
            }
        }
        struct timespec ts = {0, 1000000};
        nanosleep(&ts, NULL);
    }
}

typedef struct
{
    int x, y;
    int frame;
} Spinner;

void spinner_frame(void *data)
{
    Spinner *s = (Spinner *)data;
    char frames[] = "|/-\\";
    printf("\033[%d;%dH%c", s->y, s->x, frames[s->frame]);
    fflush(stdout);
    s->frame = (s->frame + 1) % 4;
}

typedef struct
{
    int x, y;
    int width;
    int progress;
} ProgressBar;

void progress_frame(void *data)
{
    ProgressBar *p = (ProgressBar *)data;
    p->progress = (p->progress + 1) % 101;
    int filled = (p->progress * p->width) / 100;
    printf("\033[%d;%dH[", p->y, p->x);
    for (int i = 0; i < p->width; i++)
        printf("%c", i < filled ? '=' : ' ');
    printf("] %3d%%", p->progress);
    fflush(stdout);
}

int anim_add_progress(int x, int y, int width, int progress, int fps)
{
    ProgressBar *p = malloc(sizeof(ProgressBar));
    if (!p) return -1;

    *p = (ProgressBar){x, y, width, progress};
    return anim_add(progress_frame, p, fps);
}

int anim_add_spinner(int x, int y, int fps)
{
    Spinner *s = malloc(sizeof(Spinner));
    if (!s) return -1;

    *s = (Spinner){x, y, 0};
    return anim_add(spinner_frame, s, fps);
}

typedef struct
{
    int x;
    int y;
    int width;
    int offset;
    const char *text;
} Marquee;

void marquee_frame(void *data)
{
    Marquee *m = (Marquee *)data;
size_t slen = strlen(m->text);
if (slen > INT_MAX) {
}
int len = (int)slen;
    if (len == 0) return;
    char buf[m->width + 1];
    for (int i = 0; i < m->width; i++)
    {
        buf[i] = m->text[(i + m->offset) % len];
    }
    buf[m->width] = '\0';
    printf("\033[%d;%dH%s", m->y, m->x, buf);
    fflush(stdout);
    m->offset = (m->offset + 1) % len;
}

int anim_add_marquee(int x, int y, int width, const char *text, int fps)
{
    Marquee *m = malloc(sizeof(Marquee));
    if (!m) return -1;
    *m = (Marquee){
        .x = x,
        .y = y,
        .width = width,
        .offset = 0,
        .text = text
    };
    return anim_add(marquee_frame, m, fps);
}

typedef struct
{
    int x;
    int y;
    const char *text;
    int pos;
} Typewriter;

void typewriter_frame(void *data)
{
    Typewriter *t = (Typewriter *)data;

    if (t->text[t->pos] != '\0')
    {
        printf("\033[%d;%dH%c", t->y, t->x + t->pos, t->text[t->pos]);
        fflush(stdout);
        t->pos++;
    }
}

int anim_add_typewriter(int x, int y, const char *text, int fps)
{
    Typewriter *t = malloc(sizeof(Typewriter));
    if (!t) return -1;
    *t = (Typewriter){
        .x = x,
        .y = y,
        .text = text,
        .pos = 0
    };
    return anim_add(typewriter_frame, t, fps);
}

typedef struct
{
    int heads[MATRIX_WIDTH];
    int tails[MATRIX_WIDTH];
    char symbols[MATRIX_WIDTH];
} Matrix;

static char random_char(void)
{
int r = rand() % 10;
return (char)('0' + r);
}

static void matrix_frame(void *data)
{
    Matrix *m = (Matrix *)data;
    for (int i = 0; i < MATRIX_WIDTH; i++)
    {
        if (m->heads[i] < 0 && (rand() % 100 < START_PROB))
        {
            m->heads[i] = 0;
            m->tails[i] = 0;
            m->symbols[i] = random_char();
            printf("\033[1;%dH\033[1;37m%c\033[0m", i + 1, m->symbols[i]);
        }

        if (m->heads[i] >= 0)
        {
            if (m->heads[i] < MATRIX_HEIGHT && (rand() % 100 < ADVANCE_HEAD_PROB))
            {
                int old = m->heads[i]++;
                if (old >= 0)
                    printf("\033[%d;%dH\033[0;32m%c\033[0m", old + 1, i + 1, m->symbols[i]);
                if (m->heads[i] < MATRIX_HEIGHT)
                    printf("\033[%d;%dH\033[1;37m%c\033[0m", m->heads[i] + 1, i + 1, m->symbols[i]);
            }
            if (rand() % 100 < ADVANCE_TAIL_PROB && m->tails[i] < m->heads[i])
            {
                printf("\033[%d;%dH ", m->tails[i] + 1, i + 1);
                m->tails[i]++;
            }
            if (rand() % 100 < FLICKER_PROB)
            {
                m->symbols[i] = random_char();
                for (int y = m->tails[i]; y < m->heads[i]; y++)
                {
                    if (y == m->heads[i] - 1) continue;
                    printf("\033[%d;%dH\033[0;32m%c\033[0m", y + 1, i + 1, m->symbols[i]);
                }
            }
            if (m->heads[i] >= MATRIX_HEIGHT && m->tails[i] >= MATRIX_HEIGHT)
            {
                m->heads[i] = -1;
                m->tails[i] = -1;
            }
        }
    }
    fflush(stdout);
}

int anim_add_matrix_rain(int fps)
{
srand((unsigned int)time(NULL));
    Matrix *m = malloc(sizeof(Matrix));
    if (!m) return -1;
    for (int i = 0; i < MATRIX_WIDTH; i++)
    {
        m->heads[i] = -1;
        m->tails[i] = -1;
        m->symbols[i] = ' ';
    }

    return anim_add(matrix_frame, m, fps);
}

#define GRAPH_WIDTH_MAX  80

typedef struct
{
    int x, y;
    int width;
    int height;
    int data[GRAPH_WIDTH_MAX];
} Graph;

static void graph_frame(void *data)
{
    Graph *g = (Graph *)data;
    for (int i = 0; i < g->width - 1; i++)
        g->data[i] = g->data[i + 1];
    g->data[g->width - 1] = rand() % (g->height + 1);
    for (int row = 0; row <= g->height; row++)
    {
        printf("\033[%d;%dH", g->y + g->height - row, g->x);
        for (int col = 0; col < g->width; col++)
        {
            if (g->data[col] >= row)
                printf("|");
            else
                printf(" ");
        }
    }
    fflush(stdout);
}

int anim_add_graph(int x, int y, int width, int height, int fps)
{
    if (width > GRAPH_WIDTH_MAX) width = GRAPH_WIDTH_MAX;

    Graph *g = malloc(sizeof(Graph));
    if (!g) return -1;

    *g = (Graph){
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .data = {0}
    };

    return anim_add(graph_frame, g, fps);
}
/* ==================== AUDIO SPECTRUM VISUALIZER (SIMULATED) ==================== */
typedef struct
{
    int x, y;
    int num_bars;
    int max_height;
} Spectrum;

static void spectrum_frame(void *data)
{
    Spectrum *s = (Spectrum *)data;

    // Рисуем бары (случайные высоты для симуляции)
    for (int bar = 0; bar < s->num_bars; bar++)
    {
        int height = rand() % (s->max_height + 1);

        // Рисуем столбик от низа вверх (используем █ для плотности, как в cava)
        for (int h = 0; h < s->max_height; h++)
        {
            printf("\033[%d;%dH%s", s->y + s->max_height - h, s->x + bar * 2, (h < height) ? "█" : " ");
        }
    }
    fflush(stdout);
}

int anim_add_spectrum(int x, int y, int num_bars, int max_height, int fps)
{
    Spectrum *s = malloc(sizeof(Spectrum));
    if (!s) return -1;

    *s = (Spectrum){x, y, num_bars, max_height};
    return anim_add(spectrum_frame, s, fps);
}

/* ==================== CLOCK MECHANICS (только образование часов) ==================== */

static const char* get_season(int month)
{
    const char* seasons[] = {"Winter", "Spring", "Summer", "Autumn"};
    if (month == 12 || month <= 2) return seasons[0];
    if (month >= 3 && month <= 5) return seasons[1];
    if (month >= 6 && month <= 8) return seasons[2];
    return seasons[3];
}

static const char* get_month_name(int month)
{
    const char* months[] = {"January", "February", "March", "April", "May", "June",
                           "July", "August", "September", "October", "November", "December"};
    return months[month - 1];
}

static const char* get_weekday_name(int weekday)
{
    const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday"};
    return weekdays[weekday];
}

void clock_update(ClockData *cd)
{
    if (!cd) return;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    cd->year  = timeinfo->tm_year + 1900;
    cd->month = timeinfo->tm_mon + 1;
    cd->mday  = timeinfo->tm_mday;
    cd->wday  = timeinfo->tm_wday;
    cd->hour  = timeinfo->tm_hour;
    cd->min   = timeinfo->tm_min;
    cd->sec   = timeinfo->tm_sec;

    cd->season      = get_season(cd->month);
    cd->month_name  = get_month_name(cd->month);
    cd->weekday_name= get_weekday_name(cd->wday);

    sprintf(cd->hour_str, "%02d", cd->hour);
    sprintf(cd->min_str,  "%02d", cd->min);
    sprintf(cd->sec_str,  "%02d", cd->sec);
}
