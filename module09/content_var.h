#ifndef CONTENT_VAR_H
#define CONTENT_VAR_H

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

extern const char *content[];
extern const int TOTAL_CONTENT;
void content_init(void);   // ← ВОТ ЭТО ДОБАВИТЬ

#endif
