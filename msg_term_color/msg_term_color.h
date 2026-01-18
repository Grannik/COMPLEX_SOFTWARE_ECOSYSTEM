#ifndef MSG_TERM_COLOR_H
#define MSG_TERM_COLOR_H

#define NUL 0    // Терминатор для основного вызова
#define BUF (-1) // Терминатор для функции буферизации (уникальное значение)

// Основная функция (как была)
void msg_term_color(int color1, const char* text1, ...);

// Новая: сохраняет переменные для подстановки в %d
void msg_term_color_buf(int first_var, ...);

#endif
