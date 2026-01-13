#include "common.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>

void set_terminal_mode(int enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

int kbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
}

int module_07_run(void) {
    static const char *phrases[] = {
        " Per aspera ad astra Через тернии к звёздам | ",
        " Veni, vidi, vici Пришёл, увидел, победил | ",
        " Carpe diem Лови мгновение | ",
        " Cogito ergo sum Мыслю, следовательно существую | ",
        " Memento mori Помни о смерти | ",
        " Alea iacta est Жребий брошен | ",
        " Dum spiro, spero Пока дышу, надеюсь | ",
        " Sic transit gloria mundi Так проходит мирская слава | ",
        " Errare humanum est Человеку свойственно ошибаться | ",
        " Quod licet Iovi, non licet bovi Что дозволено Юпитеру, не дозволено быку | ",
        " Tertium non datur Третьего не дано | ",
        " O tempora, o mores! О времена, о нравы! | ",
        " Panem et circenses Хлеба и зрелищ | ",
        NULL
    };
    #define DISPLAY_WIDTH 76
    static char display_buffer[DISPLAY_WIDTH + 1] = {0};
    static int initialized = 0;
    static int current_phrase = 0;
    static int phrase_pos = 0;
    if (!initialized) {
        printf("\033[2J\033[H");
        memset(display_buffer, ' ', DISPLAY_WIDTH);
        display_buffer[DISPLAY_WIDTH] = '\0';
        initialized = 1;
    }
    if (phrases[current_phrase] && phrases[current_phrase][phrase_pos]) {
        memmove(display_buffer, display_buffer + 1, DISPLAY_WIDTH - 1);
        display_buffer[DISPLAY_WIDTH - 1] = phrases[current_phrase][phrase_pos];
        phrase_pos++;
    }
    if (!phrases[current_phrase] || !phrases[current_phrase][phrase_pos]) {
        current_phrase++;
        phrase_pos = 0;
        if (!phrases[current_phrase]) current_phrase = 0;
        memmove(display_buffer, display_buffer + 1, DISPLAY_WIDTH - 1);
        display_buffer[DISPLAY_WIDTH - 1] = ' ';
    }
    printf("\033[2J\033[H");
    draw_exact_frame();
    print_subtitle_left("____[][][][][][][][\033[0m7\033[32m][][][][][][][][][][][][][][][][][][][][][][][][][][][][]_____", 2, 2);
    print_subtitle_left("marquee", 3, 6);
    printf("\033[5;6H%.*s", DISPLAY_WIDTH, display_buffer);
    fflush(stdout);
    usleep(100000);
    return 0;
}

#ifndef COMPLEX_BUILD
int main() {
    char text[] = " Per aspera ad astra. ";
    int len = strlen(text);
    set_terminal_mode(1);
    while (1) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'q' || c == 'Q') {
                break;
            }
        }
        printf("\r%s", text);
        fflush(stdout);
        char first = text[0];
        for (int i = 0; i < len - 1; i++) {
            text[i] = text[i + 1];
        }
        text[len - 1] = first;
        usleep(100000);
    }
    set_terminal_mode(0);
	printf("\033[2J\033[H");
    return 0;
}
#endif

// Compilation: gcc -Wall -Wextra -O2 -std=c99 -DSTANDALONE_BUILD -o module07 module_07.c common.c
