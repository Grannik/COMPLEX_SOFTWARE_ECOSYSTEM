#include <stdio.h>
#include "content_color_3_256.h"

int main(void)
{
    for (int i = 0; i < TOTAL_CONTENT; i++)
    {
        printf("%s\n", content[i]);
    }
    return 0;
}

// gcc module_07_standalone.c -o color_3_256
