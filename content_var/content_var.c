#include <stddef.h>
#include "content_var.h"
#include <stdio.h>

int value = 123;
char temp = -5;
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
int left = 12;
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
char letter = 'A';

static char line0 [256];
static char line1 [256];
static char line2 [256];
static char line3 [256];
static char line4 [256];
static char line5 [256];
static char line6 [256];
static char line7 [256];
static char line8 [256];
static char line9 [256];
static char line10[256];
static char line11[256];
static char line12[256];
static char line13[256];
static char line14[256];
static char line15[256];
static char line16[256];
static char line17[256];
static char line18[256];
static char line19[256];
static char line20[256];
static char line21[256];
static char line22[256];
static char line23[256];
static char line24[256];
static char line25[256];
static char line26[256];
static char line27[256];
static char line28[256];
static char line29[256];
static char line30[256];
static char line31[256];
static char line32[256];
static char line33[256];
static char line34[256];
static char line35[256];
static char line36[256];
static char line37[256];
static char line38[256];

const char *content[] = {
    line0,
    line1,
    line2,
    line3,
    line4,
    line5,
    line6,
    line7,
    line8,
    line9,
    line10,
    line11,
    line12,
    line13,
    line14,
    line15,
    line16,
    line17,
    line18,
    line19,
    line20,
    line21,
    line22,
    line23,
    line24,
    line25,
    line26,
    line27,
    line28,
    line29,
    line30,
    line31,
    line32,
    line33,
    line34,
    line35,
    line36,
    line37,
    line38
};

void content_init(void)
{
 snprintf(line0,  sizeof(line0),  "\033[36mPositive integer:     \033[0m %%d    \033[36mvalue:\033[37m %d   \033[0m", value);
 snprintf(line1,  sizeof(line1),  "\033[36mNegative integer:     \033[0m %%d    \033[36mvalue:\033[37m %d   \033[0m", temp);
 snprintf(line2,  sizeof(line2),  "\033[36mUnsigned integer:     \033[0m %%u    \033[36mvalue:\033[37m %u   \033[0m", byte);
 snprintf(line3,  sizeof(line3),  "\033[36mLong integer:         \033[0m %%ld   \033[36mvalue:\033[37m %ld  \033[0m", population);
 snprintf(line4,  sizeof(line4),  "\033[36mFloat:                \033[0m %%f    \033[36mvalue:\033[37m %f   \033[0m", (double)pi_float);
 snprintf(line5,  sizeof(line5),  "\033[36mDouble:               \033[0m %%lf   \033[36mvalue:\033[37m %f   \033[0m", pi_double);
 snprintf(line6,  sizeof(line6),  "\033[36mCharacter:            \033[0m %%c    \033[36mvalue:\033[37m %c   \033[0m", letter);
 snprintf(line7,  sizeof(line7),  "\033[36mString:               \033[0m %%s    \033[36mvalue:\033[37m %s   \033[0m", text);
 snprintf(line8,  sizeof(line8),  "\033[36mPointer:              \033[0m %%p    \033[36mvalue:\033[37m %p   \033[0m", (void *)ptr);
 snprintf(line9,  sizeof(line9),  "\033[36mOctal:                \033[0m %%o    \033[36mvalue:\033[37m %o   \033[0m", oct);
 snprintf(line10, sizeof(line10), "\033[36mHexadecimal:          \033[0m %%x    \033[36mvalue:\033[37m %x   \033[0m", hex);
 snprintf(line11, sizeof(line11), "\033[36mScientific (e):       \033[0m %%e    \033[36mvalue:\033[37m %e   \033[0m", sci);
 snprintf(line12, sizeof(line12), "\033[36mScientific (E):       \033[0m %%E    \033[36mvalue:\033[37m %E   \033[0m", sciE);
 snprintf(line13, sizeof(line13), "\033[36mGeneral (g):          \033[0m %%g    \033[36mvalue:\033[37m %g   \033[0m", gen);
 snprintf(line14, sizeof(line14), "\033[36mGeneral (G):          \033[0m %%G    \033[36mvalue:\033[37m %G   \033[0m", genG);
 snprintf(line15, sizeof(line15), "\033[36mWidth/precision:      \033[0m %%8.2f \033[36mvalue:\033[37m %8.2f\033[0m", wp);
 snprintf(line16, sizeof(line16), "\033[36mLeft align:           \033[0m %%-10d \033[36mvalue:\033[37m %-10d\033[0m", left);
 snprintf(line17, sizeof(line17), "\033[36mZero padding:         \033[0m %%08d  \033[36mvalue:\033[37m %08d \033[0m", zero);
 snprintf(line18, sizeof(line18), "\033[36mPlus sign:            \033[0m %%+d   \033[36mvalue:\033[37m %+d  \033[0m", plus);
 snprintf(line19, sizeof(line19), "\033[36mSpace sign:           \033[0m %% d   \033[36mvalue:\033[37m % d  \033[0m", space);
 snprintf(line20, sizeof(line20), "\033[36mHash (oct):           \033[0m %%#o   \033[36mvalue:\033[37m %#o  \033[0m", hash_oct);
 snprintf(line21, sizeof(line21), "\033[36mHash (hex):           \033[0m %%#x   \033[36mvalue:\033[37m %#x  \033[0m", hash_hex);
 snprintf(line22, sizeof(line22), "\033[36mHash (HEX):           \033[0m %%#X   \033[36mvalue:\033[37m %#X  \033[0m", hash_HEX);
 snprintf(line23, sizeof(line23), "\033[36mInteger (i):          \033[0m %%i    \033[36mvalue:\033[37m %i   \033[0m", integer_i);
 snprintf(line24, sizeof(line24), "\033[36mUppercase hex:        \033[0m %%X    \033[36mvalue:\033[37m %X   \033[0m", upper_hex);
 snprintf(line25, sizeof(line25), "\033[36mFloat (F):            \033[0m %%F    \033[36mvalue:\033[37m %F   \033[0m", (double)float_F);
 snprintf(line26, sizeof(line26), "\033[36mPointer (nil):        \033[0m %%p    \033[36mvalue:\033[37m %p   \033[0m", nil_ptr);
 snprintf(line27, sizeof(line27), "\033[36mPercent sign:         \033[0m %%     \033[36mvalue:\033[37m %%   \033[0m");
 snprintf(line28, sizeof(line28), "\033[36mVariable width:       \033[0m\033[31m %%.*f\033[0m \033[36m (not supported)\033[0m");
 snprintf(line29, sizeof(line29), "\033[36mCount (n):            \033[0m\033[31m %%n  \033[0m \033[36m (not supported)\033[0m");
 snprintf(line30, sizeof(line30), "\033[36mHexadecimal float:    \033[0m\033[31m %%a  \033[0m \033[36m (not supported)\033[0m");
 snprintf(line31, sizeof(line31), "\033[36mHexadecimal float (A):\033[0m\033[31m %%A  \033[0m \033[36m (not supported)\033[0m");
 snprintf(line32, sizeof(line32), "\033[36mLong long:            \033[0m\033[31m %%lld\033[0m \033[36m (not supported)\033[0m");
 snprintf(line33, sizeof(line33), "\033[36mUnsigned long long:   \033[0m\033[31m %%llu\033[0m \033[36m (not supported)\033[0m");
 snprintf(line34, sizeof(line34), "\033[36mSigned char:          \033[0m\033[31m %%hhd\033[0m \033[36m (not supported)\033[0m");
 snprintf(line35, sizeof(line35), "\033[36mShort integer:        \033[0m\033[31m %%hd \033[0m \033[36m (not supported)\033[0m");
 snprintf(line36, sizeof(line36), "\033[36mUnsigned long:        \033[0m\033[31m %%lu \033[0m \033[36m (not supported)\033[0m");
 snprintf(line37, sizeof(line37), "\033[36mUnsigned short:       \033[0m\033[31m %%hu \033[0m \033[36m (not supported)\033[0m");
 snprintf(line38, sizeof(line38), "\033[36mLong double:          \033[0m\033[31m %%Lf \033[0m \033[36m (not supported)\033[0m");
}

const int TOTAL_CONTENT = sizeof(content) / sizeof(content[0]);
