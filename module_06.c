#include "common.h"
#include <time.h> //---
#include <string.h>
#include <unistd.h>
#include <time.h>

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

int module_06_run(void) {
    static char text[] = " Per aspera ad astra. ";
    static int initialized = 0;
    if (!initialized) {
        printf("\033[2J\033[H");
        initialized = 1;
    }
    int len = strlen(text);
    char first = text[0];
    memmove(text, text + 1, len - 1);
    text[len - 1] = first;
    printf("\033[2;2H%s", text);
    fflush(stdout);
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
    printf("\nПрограмма завершена.\n");
    return 0;
}
#endif

// gcc -Wall -Wextra -O2 -std=c99 -DSTANDALONE_BUILD -o module06 module_06.c common.c
