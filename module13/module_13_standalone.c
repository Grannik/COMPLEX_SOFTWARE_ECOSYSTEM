#include <stdio.h>
#include <stdlib.h>

#include "../libtermcontrol/libtermcontrol.h"
#include "../libtermcolor/libtermcolor.h"
#include "../libtermcolor/tc_file_types.h"
#include "../libtermcolor/tc_file_directories_color.h"

void print_file_content(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\033[1;31m Error:\033[0m cannot open file %s\n", filename);
        return;
    }
    int c;
    while ((c = fgetc(file)) != EOF) putchar(c);
    fclose(file);
}

int handle_view_request(const char *filename) {
    if (is_file_category(filename, FILE_CATEGORY_TEXT)) {
       term_screen_restore_full();
       print_file_content(filename);
       return 0;
    }
    FileType type = get_file_type(filename);
    const char *type_name = get_file_type_name(type);
    printf("\033[1;31m Error:\033[0m File '%s' is not a text file (Type: %s). Access denied.\n", filename, type_name);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    return handle_view_request(argv[1]);
}

// clear;gcc module_13_standalone.c ../libtermcontrol/libtermcontrol.c ../libtermcolor/*.c -Ilibtermcolor -o view_text -lncursesw
