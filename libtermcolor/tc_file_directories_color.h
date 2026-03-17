#ifndef TC_FILE_DIRECTORIES_COLOR_H
#define TC_FILE_DIRECTORIES_COLOR_H

#include "tc_file_types.h"

int extension_in_list(const char *ext, const char *const *list);
FileType get_file_type(const char *filename);
const char *get_file_type_name(FileType type);
int is_file_type(const char *filename, FileType type);
int is_file_category(const char *filename, unsigned int category_flags);
int get_text_module_file_color(const char *filename, int is_dir, char flag);
int get_analyzer_file_color(const char *filename, int is_dir);

#endif
