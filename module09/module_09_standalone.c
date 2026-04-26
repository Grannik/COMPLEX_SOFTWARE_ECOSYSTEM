#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "content_var.h"

int main(void)
{
    content_init();
    system("clear");
    for (int i = 0; i < TOTAL_CONTENT; i++) {
        printf(" %s\n", content[i]);
    }
    return 0;
}

// clear;gcc module_09_standalone.c content_var.c -o module_09_standalone
