#include <stddef.h>
#include "content_var.h"

int value = 123;
char temp = -5;
char letter = 'A';
unsigned char byte = 255;
short small = -32768;
long population = 7800000000L;
float pi_float = 3.14159f;
double pi_double = 3.141592653589793;
double sci = 123456.789;
double sciE = 9876.54321;
double gen = 123.456789;
double genG = 0.000123456;
double wp = 3.14159;
const char* text = "Hello";
int var = 42;
int* ptr = &var;
int oct = 255;
int hex = 0xABC;
int left = 42;
int zero = 123;
int plus = 2;
int space = 7;
int hash_oct = 255;
int hash_hex = 255;
int hash_HEX = 255;
int integer_i = -19;
int upper_hex = 0xABC;
float float_F = 3.14f;
void* nil_ptr = NULL;

void line_00(void) {
    libtermcolor_buf_int(value);
    libtermcolor_msg(36,"  Positive integer:     ", 37," %%d  ",
                     36,"   value:", 37," %d", NUL);
}

void line_01(void) {
    libtermcolor_buf_int((int)temp);
    libtermcolor_msg(36,"  Negative integer:     ", 37," %%d  ",
                     36,"   value:", 37," %d", NUL);
}

void line_02(void) {
    libtermcolor_buf_unsigned((unsigned int)byte);
    libtermcolor_msg(36,"  Unsigned integer:     ", 37," %%u  ",
                     36,"   value:", 37," %u", NUL);
}

void line_03(void) {
    libtermcolor_buf_long(population);
    libtermcolor_msg(36,"  Long integer:         ", 37," %%ld ", 36,"   value:", 37," %ld", NUL);
}

void line_04(void) {
    libtermcolor_buf_double((double)pi_float);
    libtermcolor_msg(36,"  Float:                ", 37," %%f  ", 36,"   value:", 37," %f", NUL);
}

void line_05(void) {
    libtermcolor_buf_double(pi_double);
    libtermcolor_msg(36,"  Double:               ", 37," %%lf ", 36,"   value:", 37," %lf", NUL);
}

void line_06(void) {
    libtermcolor_buf_char(letter);
    libtermcolor_msg(36,"  Character:            ", 37," %%c  ", 36,"   value:", 37," %c", NUL);
}

void line_07(void) {
    libtermcolor_buf_string(text);
    libtermcolor_msg(36,"  String:               ", 37," %%s  ", 36,"   value:", 37," %s", NUL);
}

void line_08(void) {
    libtermcolor_buf_ptr((void*)ptr);
    libtermcolor_msg(36,"  Pointer:              ", 37," %%p  ", 36,"   value:", 37," %p", NUL);
}

void line_09(void) {
    libtermcolor_buf_int(oct);
    libtermcolor_msg(36,"  Octal:                ", 37," %%o  ", 36,"   value:", 37," %o", NUL);
}

void line_10(void) {
    libtermcolor_buf_int(hex);
    libtermcolor_msg(36,"  Hexadecimal:          ", 37," %%x   ", 36,"  value:", 37," %x", NUL);
}

void line_11(void) {
    libtermcolor_buf_double(sci);
    libtermcolor_msg(36,"  Scientific (e):       ", 37," %%e   ", 36,"  value:", 37," %e", NUL);
}

void line_12(void) {
    libtermcolor_buf_double(sciE);
    libtermcolor_msg(36,"  Scientific (E):       ", 37," %%E   ", 36,"  value:", 37," %E", NUL);
}

void line_13(void) {
    libtermcolor_buf_double(gen);
    libtermcolor_msg(36,"  General (g):          ", 37," %%g   ", 36,"  value:", 37," %g", NUL);
}

void line_14(void) {
    libtermcolor_buf_double(genG);
    libtermcolor_msg(36,"  General (G):          ", 37," %%G   ", 36,"  value:", 37," %G", NUL);
}

void line_15(void) {
    libtermcolor_buf_double(wp);
    libtermcolor_msg(36,"  Width/precision:      ", 37," %%8.2f", 36,"  value:", 37," %8.2f", NUL);
}

void line_16(void) {
    libtermcolor_buf_int(left);
    libtermcolor_msg(36,"  Left align:           ", 37," %%-10d", 36,"  value:", 37," %-10d", NUL);
}

void line_17(void) {
    libtermcolor_buf_int(zero);
    libtermcolor_msg(36,"  Zero padding:         ", 37," %%08d ", 36,"  value:", 37," %08d", NUL);
}

void line_18(void) {
    libtermcolor_buf_int(plus);
    libtermcolor_msg(36,"  Plus sign:            ", 37," %%+d  ", 36,"  value:", 37," %+d", NUL);
}

void line_19(void) {
    libtermcolor_buf_int(space);
    libtermcolor_msg(36,"  Space sign:           ", 37," %% d  ", 36,"  value:", 37," % d", NUL);
}

void line_20(void) {
    libtermcolor_buf_int(hash_oct);
    libtermcolor_msg(36,"  Hash (oct):           ", 37," %%#o  ", 36,"  value:", 37," %#o", NUL);
}

void line_21(void) {
    libtermcolor_buf_int(hash_hex);
    libtermcolor_msg(36,"  Hash (hex):           ", 37," %%#x  ", 36,"  value:", 37," %#x", NUL);
}

void line_22(void) {
    libtermcolor_buf_int(hash_HEX);
    libtermcolor_msg(36,"  Hash (HEX):           ", 37," %%#X  ", 36,"  value:", 37," %#X", NUL);
}

void line_23(void) {
    libtermcolor_buf_int(integer_i);
    libtermcolor_msg(36,"  Integer (i):          ", 37," %%i   ", 36,"  value:", 37," %i", NUL);
}

void line_24(void) {
    libtermcolor_buf_int(upper_hex);
    libtermcolor_msg(36,"  Uppercase hex:        ", 37," %%X   ", 36,"  value:", 37," %X", NUL);
}

void line_25(void) {
    libtermcolor_buf_double((double)float_F);
    libtermcolor_msg(36,"  Float (F):            ", 37," %%F   ", 36,"  value:", 37," %F", NUL);
}

void line_26(void) {
    libtermcolor_buf_ptr(nil_ptr);
    libtermcolor_msg(36,"  Pointer (nil):        ", 37," %%p   ", 36,"  value:", 37," %p", NUL);
}

void line_27(void) {
    libtermcolor_msg(36,"  Percent sign:         ", 37," %     ", 36," value:", 37," %", NUL);
}

void line_28(void) {
    libtermcolor_msg(36,"  Variable width:       ", 31," %.*f  ", 36," (not supported)", NUL);
}

void line_29(void) {
    libtermcolor_msg(36,"  Count (n):            ", 31," %n    ", 36," (not supported)", NUL);
}

void line_30(void) {
    libtermcolor_msg(36,"  Hexadecimal float:    ", 31," %a    ", 36," (not supported)", NUL);
}

void line_31(void) {
    libtermcolor_msg(36,"  Hexadecimal float (A):", 31," %A    ", 36," (not supported)", NUL);
}

void line_32(void) {
    libtermcolor_msg(36,"  Long long:            ", 31," %lld  ", 36," (not supported)", NUL);
}

void line_33(void) {
    libtermcolor_msg(36,"  Unsigned long long:   ", 31," %llu  ", 36," (not supported)", NUL);
}

void line_34(void) {
    libtermcolor_msg(36,"  Signed char:          ", 31," %hhd  ", 36," (not supported)", NUL);
}

void line_35(void) {
    libtermcolor_msg(36,"  Short integer:        ", 31," %hd   ", 36," (not supported)", NUL);
}

void line_36(void) {
    libtermcolor_msg(36,"  Unsigned long:        ", 31," %lu   ", 36," (not supported)", NUL);
}

void line_37(void) {
    libtermcolor_msg(36,"  Unsigned short:       ", 31," %hu   ", 36," (not supported)", NUL);
}

void line_38(void) {
    libtermcolor_msg(36,"  Long double:          ", 31," %Lf   ", 36," (not supported)", NUL);
}

//
void (*content[])(void) = {
    line_00,
    line_01,
    line_02,
    line_03,
    line_04,
    line_05,
    line_06,
    line_07,
    line_08,
    line_09,
    line_10,
    line_11,
    line_12,
    line_13,
    line_14,
    line_15,
    line_16,
    line_17,
    line_18,
    line_19,
    line_20,
    line_21,
    line_22,
    line_23,
    line_24,
    line_25,
    line_26,
    line_27,
    line_28,
    line_29,
    line_30,
    line_31,
    line_32,
    line_33,
    line_34,
    line_35,
    line_36,
    line_37,
    line_38,
};

const int TOTAL_CONTENT = sizeof(content) / sizeof(content[0]);
