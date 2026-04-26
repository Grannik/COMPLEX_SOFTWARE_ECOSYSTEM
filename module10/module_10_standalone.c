#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "content_abc.h"

int main(void)
{
    system("clear");
    for (int i = 0; i < ABC_TOTAL_CONTENT; i++) {
        printf("\033[35m %s\033[0m\n", abc_content[i]);
    }
    return 0;
}

// clear; gcc module_10_standalone.c -o module_10_standalone -std=c11 -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wundef -Wstrict-prototypes -Wmissing-prototypes -Wcast-align -Wcast-qual -Wwrite-strings -Wswitch-enum -Wuninitialized -Winit-self -Wfloat-equal -fno-common -fsanitize=address,undefined -fno-omit-frame-pointer -g
