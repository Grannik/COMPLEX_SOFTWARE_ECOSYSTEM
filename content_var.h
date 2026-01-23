#ifndef CONTENT_VAR_H
#define CONTENT_VAR_H

#include "libtermcolor/libtermcolor.h"

//#define TOTAL_CONTENT (sizeof(content) / sizeof(content[0]))

// Объявления переменных
extern int value;
extern char temp;
extern char letter;
extern unsigned char byte;
extern short small;
extern long population;
extern float pi_float;
extern double pi_double;
extern double sci;
extern double sciE;
extern double gen;
extern double genG;
extern double wp;
extern const char* text;
extern int var;
extern int* ptr;
extern int oct;
extern int hex;
extern int left;
extern int zero;
extern int plus;
extern int space;
extern int hash_oct;
extern int hash_hex;
extern int hash_HEX;
extern int integer_i;
extern int upper_hex;
extern float float_F;
extern void* nil_ptr;

// Прототипы функций
void line_00(void);
void line_01(void);
void line_02(void);
void line_03(void);
void line_04(void);
void line_05(void);
// ... добавь прототипы для всех line_XX, сколько нужно

// Объявление массива функций
extern void (*content[])(void);
extern const int TOTAL_CONTENT;  // добавить
/*
typedef enum {
    DISPLAY_LINES,
    DISPLAY_FUNCS
} display_mode_t;

void display_from_line_mode(int start_line, display_mode_t mode);
*/
#endif
