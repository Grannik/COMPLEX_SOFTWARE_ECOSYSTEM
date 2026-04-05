#define _GNU_SOURCE
#include "analyzer_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

int mod_01_run(const char *filename);
int mod_02_run(const char *filename);
int mod_03_run(const char *filename);
int mod_04_run(const char *filename);
int mod_05_run(const char *filename);
int mod_06_run(const char *filename);
int mod_07_run(const char *filename);
int mod_08_run(const char *filename);
int mod_09_run(const char *filename);
int mod_10_run(const char *filename);
int mod_11_run(const char *filename);

static int (*module_runs[])(const char *) = {
    mod_01_run,
    mod_02_run,
    mod_03_run,
    mod_04_run,
    mod_05_run,
    mod_06_run,
    mod_07_run,
    mod_08_run,
    mod_09_run,
    mod_10_run,
    mod_11_run,
};

static const char *module_names[] = {
    "01: Regular file check",
    "02: Text content analysis",
    "03: Extended file statistics",
    "04: MIME type (libmagic)",
    "05: UTF-8 check (iconv)",
    "06: Encoding detection (uchardet)",
    "07: Archive check (libarchive)",
    "08: Media analysis (libavformat)",
    "09: Signature check",
    "10: Basic libc analysis",
    "11: EXIF data (libexif)",
};

#define MODULE_COUNT (sizeof(module_runs) / sizeof(module_runs[0]))

void analysis_result_init(AnalysisResult *result) {
    if (!result) return;
    result->lines = NULL;
    result->line_count = 0;
    result->capacity = 0;
}

void analysis_result_add_line(AnalysisResult *result, const char *line) {
    if (!result || !line) return;
    if (result->line_count >= result->capacity) {
        int new_capacity = result->capacity == 0 ? 64 : result->capacity * 2;
        char **new_lines = realloc(result->lines, (size_t)new_capacity * sizeof(char*));
        if (!new_lines) return;
        result->lines = new_lines;
        result->capacity = new_capacity;
    }
    result->lines[result->line_count] = strdup(line);
    if (result->lines[result->line_count]) {
        result->line_count++;
    }
}

void analysis_result_free(AnalysisResult *result) {
    if (!result) return;
    for (int i = 0; i < result->line_count; i++) {
        free(result->lines[i]);
    }
    free(result->lines);
    result->lines = NULL;
    result->line_count = 0;
    result->capacity = 0;
}

static int capture_output_to_tempfile(const char *filepath, char ***lines, int *line_count) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return -1;
    }
    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1 ||
            dup2(pipefd[1], STDERR_FILENO) == -1) {
            exit(1);
        }
        close(pipefd[1]);
        for (size_t i = 0; i < MODULE_COUNT; i++) {
            printf("\n" " Module: %s\n", module_names[i]);
            fflush(stdout);
            module_runs[i](filepath);
            fflush(stdout);
        }
        exit(0);
    } else {
        close(pipefd[1]);
        FILE *output = fdopen(pipefd[0], "r");
        if (!output) {
            close(pipefd[0]);
            return -1;
        }
        char buffer[1024];
        int capacity = 64;
        int count = 0;
        char **result_lines = malloc((size_t)capacity * sizeof(char*));
        while (fgets(buffer, sizeof(buffer), output)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
            }
            if (buffer[0] == '\0') continue;
            if (count >= capacity) {
                capacity *= 2;
                char **new_lines = realloc(result_lines, (size_t)capacity * sizeof(char*));
                if (!new_lines) {
                    for (int j = 0; j < count; j++) free(result_lines[j]);
                    free(result_lines);
                    fclose(output);
                    return -1;
                }
                result_lines = new_lines;
            }
            result_lines[count] = strdup(buffer);
            count++;
        }
        fclose(output);
        int status;
        waitpid(pid, &status, 0);
        *lines = result_lines;
        *line_count = count;
        return 0;
    }
}

int run_file_analysis(const char *filepath, AnalysisResult *result) {
    if (!filepath || !result) return -1;
    if (access(filepath, F_OK) != 0) {
        analysis_result_add_line(result, "Error: File does not exist");
        return -1;
    }
    char **lines = NULL;
    int line_count = 0;
    if (capture_output_to_tempfile(filepath, &lines, &line_count) != 0) {
        analysis_result_add_line(result, "Error: Failed to run analysis");
        return -1;
    }
    for (int i = 0; i < line_count; i++) {
        analysis_result_add_line(result, lines[i]);
        free(lines[i]);
    }
    free(lines);
    return 0;
}

int get_analyzer_module_count(void) {
    return MODULE_COUNT;
}

const char *get_analyzer_module_name(int module_index) {
    if (module_index >= 0 && (size_t)module_index < MODULE_COUNT) {
        return module_names[module_index];
    }
    return "Unknown";
}
