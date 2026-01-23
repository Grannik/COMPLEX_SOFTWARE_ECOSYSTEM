#include "libtermcolor.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

typedef union {
    int i;
    unsigned int u;
    long l;
    unsigned long ul;
    long long ll;
    double d;
    char c;
    const char* s;
    void* p;
} ValueUnion;

typedef struct {
    int type;
    ValueUnion value;
} BufferItem;

static BufferItem var_buffer[32];
static int var_count = 0;
static int var_index = 0;

static const char* color_to_ansi(int color_code) {
    if (color_code >= 5001 && color_code <= 5255) {
        static __thread char buf[16];
        snprintf(buf, sizeof(buf), "\033[38;5;%dm", color_code - 5000);
        return buf;
    }

    switch (color_code) {
        case 0:   return "\033[0m";
        case 1:   return "\033[1m";
        case 2:   return "\033[2m";
        case 3:   return "\033[3m";
        case 4:   return "\033[4m";
        case 5:   return "\033[5m";
        case 7:   return "\033[7m";
        case 8:   return "\033[8m";
        case 9:   return "\033[9m";
        case 22:  return "\033[22m";
        case 23:  return "\033[23m";
        case 24:  return "\033[24m";
        case 25:  return "\033[25m";
        case 27:  return "\033[27m";
        case 28:  return "\033[28m";
        case 29:  return "\033[29m";
        case 30:  return "\033[30m";
        case 31:  return "\033[31m";
        case 32:  return "\033[32m";
        case 33:  return "\033[33m";
        case 34:  return "\033[34m";
        case 35:  return "\033[35m";
        case 36:  return "\033[36m";
        case 37:  return "\033[37m";
        case 40:  return "\033[40m";
        case 41:  return "\033[41m";
        case 42:  return "\033[42m";
        case 43:  return "\033[43m";
        case 44:  return "\033[44m";
        case 45:  return "\033[45m";
        case 46:  return "\033[46m";
        case 47:  return "\033[47m";
        case 90:  return "\033[90m";
        case 91:  return "\033[91m";
        case 92:  return "\033[92m";
        case 93:  return "\033[93m";
        case 94:  return "\033[94m";
        case 95:  return "\033[95m";
        case 96:  return "\033[96m";
        case 97:  return "\033[97m";
        case 100: return "\033[100m";
        case 101: return "\033[101m";
        case 102: return "\033[102m";
        case 103: return "\033[103m";
        case 104: return "\033[104m";
        case 105: return "\033[105m";
        case 106: return "\033[106m";
        case 107: return "\033[107m";
        default:  return "\033[0m";
    }
}

static void format_with_buffer_simple(const char* src, char* dest, size_t dest_size) {
    size_t src_len = strlen(src);
    size_t dest_idx = 0;
    for (size_t i = 0; i < src_len && dest_idx < dest_size - 1; i++) {
        if (src[i] == '%' && i + 1 < src_len) {
            if (src[i + 1] == '%') {
                if (dest_idx < dest_size - 1) {
                    dest[dest_idx++] = '%';
                }
                i++;
                continue;
            }
            char specifier[32] = {0};
            size_t spec_len = 0;
            specifier[spec_len++] = '%';
            int is_long = 0;
            int is_long_long = 0;
            for (size_t j = i + 1; j < src_len && spec_len < sizeof(specifier) - 1; j++) {
                char c = src[j];
                specifier[spec_len++] = c;
                if (c == 'l') {
                    if (j + 1 < src_len && src[j + 1] == 'l') {
                        is_long_long = 1;
                        specifier[spec_len++] = 'l';
                        j++;
                    } else {
                        is_long = 1;
                    }
                    continue;
                }
                if (c == 'd' || c == 'i' || c == 'u' || c == 'o' ||
                    c == 'x' || c == 'X' || c == 'f' || c == 'F' ||
                    c == 'e' || c == 'E' || c == 'g' || c == 'G' ||
                    c == 'c' || c == 's' || c == 'p') {
                    break;
                }
                if (c == '.' || c == '-' || c == '+' || c == ' ' ||
                    c == '#' || c == '0' || (c >= '1' && c <= '9')) {
                    continue;
                }
                spec_len--;
                break;
            }
            specifier[spec_len] = '\0';
            char format_char = specifier[spec_len - 1];
            if (var_index < var_count) {
                BufferItem* item = &var_buffer[var_index];
                char formatted[128];
                int handled = 1;
                if (format_char == 'd' || format_char == 'i' ||
                    format_char == 'u' || format_char == 'o' ||
                    format_char == 'x' || format_char == 'X') {
                    if (is_long_long) {
                        long long val = 0;
                        if (item->type == 0) val = (long long)item->value.i;
                        else if (item->type == 1) val = (long long)item->value.u;
                        else if (item->type == 2) val = (long long)item->value.l;
                        else handled = 0;
                        if (handled) snprintf(formatted, sizeof(formatted), specifier, val);
                    }
                    else if (is_long) {
                        if (format_char == 'u' || format_char == 'o' ||
                            format_char == 'x' || format_char == 'X') {
                            unsigned long val = 0;
                            if (item->type == 0) val = (unsigned long)item->value.i;
                            else if (item->type == 1) val = (unsigned long)item->value.u;
                            else if (item->type == 2) val = (unsigned long)item->value.l;
                            else handled = 0;
                            if (handled) snprintf(formatted, sizeof(formatted), specifier, val);
                        } else {
                            long val = 0;
                            if (item->type == 0) val = (long)item->value.i;
                            else if (item->type == 1) val = (long)item->value.u;
                            else if (item->type == 2) val = item->value.l;
                            else handled = 0;
                            if (handled) snprintf(formatted, sizeof(formatted), specifier, val);
                        }
                    }
                    else {
                        if (format_char == 'u' || format_char == 'o' ||
                            format_char == 'x' || format_char == 'X') {
                            unsigned int val = 0;
                            if (item->type == 0) val = (unsigned int)item->value.i;
                            else if (item->type == 1) val = item->value.u;
                            else if (item->type == 2) val = (unsigned int)item->value.l;
                            else handled = 0;
                            if (handled) snprintf(formatted, sizeof(formatted), specifier, val);
                        } else {
                            int val = 0;
                            if (item->type == 0) val = item->value.i;
                            else if (item->type == 1) val = (int)item->value.u;
                            else if (item->type == 2) val = (int)item->value.l;
                            else handled = 0;
                            if (handled) snprintf(formatted, sizeof(formatted), specifier, val);
                        }
                    }
                }
                else if (format_char == 'f' || format_char == 'F' ||
                         format_char == 'e' || format_char == 'E' ||
                         format_char == 'g' || format_char == 'G') {
                    if (item->type == 3) {
                        snprintf(formatted, sizeof(formatted), specifier, item->value.d);
                    }
                    else if (item->type == 0 || item->type == 1) {
                        double val = (item->type == 0) ? 
                                   (double)item->value.i : (double)item->value.u;
                        snprintf(formatted, sizeof(formatted), specifier, val);
                    }
                    else {
                        handled = 0;
                    }
                }
                else if (format_char == 'c') {
                    if (item->type == 4) {
                        snprintf(formatted, sizeof(formatted), specifier, item->value.c);
                    }
                    else if (item->type == 0) {
                        snprintf(formatted, sizeof(formatted), specifier, (char)item->value.i);
                    }
                    else {
                        handled = 0;
                    }
                }
                else if (format_char == 's') {
                    if (item->type == 5) {
                        snprintf(formatted, sizeof(formatted), specifier, 
                                item->value.s ? item->value.s : "(null)");
                    }
                    else {
                        handled = 0;
                    }
                }
                else if (format_char == 'p') {
                    if (item->type == 6) {
                        snprintf(formatted, sizeof(formatted), specifier, item->value.p);
                    }
                    else {
                        handled = 0;
                    }
                }
                else {
                    handled = 0;
                }
                if (handled) {
                    size_t fmt_len = strlen(formatted);
                    if (dest_idx + fmt_len < dest_size) {
                        strcpy(&dest[dest_idx], formatted);
                        dest_idx += fmt_len;
                    }
                    var_index++;
                } else {
                    if (dest_idx + spec_len < dest_size) {
                        strncpy(&dest[dest_idx], specifier, spec_len);
                        dest_idx += spec_len;
                    }
                }
                i += spec_len - 1;
            } else {
                if (dest_idx + spec_len < dest_size) {
                    strncpy(&dest[dest_idx], specifier, spec_len);
                    dest_idx += spec_len;
                }
                i += spec_len - 1;
            }
        } else {
            dest[dest_idx++] = src[i];
        }
    }
    dest[dest_idx] = '\0';
}

void libtermcolor_msg(int color1, const char* text1, ...) {
    va_list ap;
    va_start(ap, text1);
    var_index = 0;
    const char* color_code = color_to_ansi(color1);
    fprintf(stderr, "%s", color_code);
    if (var_count > 0) {
        char formatted[512];
        format_with_buffer_simple(text1, formatted, sizeof(formatted));
        fprintf(stderr, "%s", formatted);
    } else {
        fprintf(stderr, "%s", text1);
    }
    int expecting_color = 0;
    while (1) {
        if (!expecting_color) {
            int next_color = va_arg(ap, int);
            if (next_color == NUL) break;
            color_code = color_to_ansi(next_color);
            fprintf(stderr, "%s", color_code);
            expecting_color = 1;
        } else {
            const char* next_text = va_arg(ap, const char*);
            if (var_count > 0) {
                char formatted[512];
                format_with_buffer_simple(next_text, formatted, sizeof(formatted));
                fprintf(stderr, "%s", formatted);
            } else {
                fprintf(stderr, "%s", next_text);
            }
            expecting_color = 0;
        }
    }
    var_count = 0;
    var_index = 0;
    fprintf(stderr, "\033[0;49m\n");
    va_end(ap);
}

void libtermcolor_buf_int(int value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 0;
        var_buffer[var_count].value.i = value;
        var_count++;
    }
}

void libtermcolor_buf_unsigned(unsigned int value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 1;
        var_buffer[var_count].value.u = value;
        var_count++;
    }
}

void libtermcolor_buf_double(double value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 3;
        var_buffer[var_count].value.d = value;
        var_count++;
    }
}

void libtermcolor_buf_string(const char* value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 5;
        var_buffer[var_count].value.s = value;
        var_count++;
    }
}

void libtermcolor_buf_ptr(void* value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 6;
        var_buffer[var_count].value.p = value;
        var_count++;
    }
}

void libtermcolor_buf_char(char value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 4;
        var_buffer[var_count].value.c = value;
        var_count++;
    }
}

void libtermcolor_buf_long(long value) {
    if (var_count < 32) {
        var_buffer[var_count].type = 2;
        var_buffer[var_count].value.l = value;
        var_count++;
    }
}

void libtermcolor_buf(int first_var, ...) {
    va_list ap;
    va_start(ap, first_var);
    var_count = 0;
    int arg = first_var;
    while (arg != BUF && var_count < 32) {
        var_buffer[var_count].type = 0;
        var_buffer[var_count].value.i = arg;
        var_count++;
        arg = va_arg(ap, int);
    }
    var_index = 0;
    va_end(ap);
}

void libtermcolor_pos(int arg1, ...) {
    va_list ap;
    va_start(ap, arg1);
    int x = arg1;
    int y = va_arg(ap, int);
    int separator = va_arg(ap, int);
    if (separator != POS) {
        fprintf(stderr, "ERROR: Missing POS separator\n");
        va_end(ap);
        return;
    }
    printf("\033[%d;%dH", y, x);
    int expecting_color = 1;
    int current_color;
    while (1) {
        if (expecting_color) {
            current_color = va_arg(ap, int);
            if (current_color == NUL) break;
            printf("%s", color_to_ansi(current_color));
            expecting_color = 0;
        } else {
            const char* text = va_arg(ap, const char*);
            printf("%s", text ? text : "(null)");
            expecting_color = 1;
        }
    }
    printf("\033[0m");
    fflush(stdout);
    va_end(ap);
}

// 369
