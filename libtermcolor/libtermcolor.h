#ifndef LIBMSGTERMCOLOR_H
#define LIBMSGTERMCOLOR_H

#define NUL 0
#define BUF (-1)
#define POS (-2)

void libtermcolor_msg(int color1, const char* text1, ...);
void libtermcolor_buf(int first_var, ...);

void libtermcolor_buf_int(int value);
void libtermcolor_buf_unsigned(unsigned int value);
void libtermcolor_buf_long(long value);
void libtermcolor_buf_double(double value);
void libtermcolor_buf_char(char value);
void libtermcolor_buf_string(const char* value);
void libtermcolor_buf_ptr(void* value);
void libtermcolor_cleanup(void);
void libtermcolor_pos(int arg1, ...);

#endif
