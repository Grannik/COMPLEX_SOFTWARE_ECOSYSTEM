#ifndef ANALYZER_WRAPPER_H
#define ANALYZER_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char **lines;
    int line_count;
    int capacity;
} AnalysisResult;

void analysis_result_init(AnalysisResult *result);
void analysis_result_add_line(AnalysisResult *result, const char *line);
void analysis_result_free(AnalysisResult *result);
int run_file_analysis(const char *filepath, AnalysisResult *result);
int get_analyzer_module_count(void);
const char *get_analyzer_module_name(int module_index);

#endif
