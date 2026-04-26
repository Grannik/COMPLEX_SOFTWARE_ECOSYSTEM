#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer_wrapper.h"
#include "../libtermcontrol/libtermcontrol.h"
#include "../libtermcolor/libtermcolor.h"

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <file>\n", prog_name);
}

void print_result_line(const char *line) {
    if (!line) return;
    const char *color_module = "\033[1;35m";
    const char *color_reset = "\033[0m";
    if (strncmp(line, " Module:", 8) == 0) {
        printf("%s%s%s\n", color_module, line, color_reset);
    } else {
        printf("  %s\n", line);
    }
}

void display_analysis_report(const AnalysisResult *result) {
    for (int i = 0; i < result->line_count; i++) {
        print_result_line(result->lines[i]);
    }
}

int run_app_logic(const char *filepath) {
    AnalysisResult result;
    analysis_result_init(&result);
    term_screen_clear_and_home();
    printf("\033[1;2H");
    if (run_file_analysis(filepath, &result) != 0) {
        fprintf(stderr, "Error: Failed to analyze file: %s\n", filepath);
        analysis_result_free(&result);
        return -1;
    }
    display_analysis_report(&result);
    analysis_result_free(&result);
        printf("\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (run_app_logic(argv[1]) != 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// gcc module_12_standalone.c analyzer_wrapper.c file_analyzer.c ../libtermcontrol/libtermcontrol.c ../libtermcolor/*.c -o file_analyzer -lmagic -luchardet -larchive -lavformat -lavcodec -lavutil -lexif -lz -lbz2 -llzma -lm -lncurses -ltinfo
