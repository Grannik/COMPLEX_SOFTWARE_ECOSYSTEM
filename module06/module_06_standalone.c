#include <stdio.h>
#include "content_color_16.h"

int main(void)
{
    for (int i = 0; i < TOTAL_CONTENT; i++)
    {
        printf("%s\n", content[i]);
    }

    return 0;
}

// gcc module_06_standalone.c -o color_16
