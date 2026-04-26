void clear_screen(void);
int anim_add(void (*frame)(void *data), void *data, int fps);
void anim_loop(void);

int anim_add_spinner(int x, int y, int fps);
int anim_add_progress(int x, int y, int width, int progress, int fps);
int anim_add_marquee(int x, int y, int width, const char *text, int fps);
int anim_add_typewriter(int x, int y, const char *text, int fps);
int anim_add_matrix_rain(int fps);
int anim_add_graph(int x, int y, int width, int height, int fps);
int anim_add_spectrum(int x, int y, int num_bars, int max_height, int fps);

typedef struct {
    int year;           // 1900 + tm_year
    int month;          // 1-12
    int mday;           // день месяца
    int wday;           // день недели (0=воскресенье)
    int hour;
    int min;
    int sec;

    const char* season;      // "Winter", "Spring"...
    const char* month_name;  // "January"...
    const char* weekday_name;// "Sunday"...

    char hour_str[3];   // "23"
    char min_str[3];    // "59"
    char sec_str[3];    // "59"
} ClockData;

void clock_update(ClockData *cd);
