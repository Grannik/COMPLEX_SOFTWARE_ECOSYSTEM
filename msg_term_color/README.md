
# msg_term_color – Библиотека цветного вывода в терминал на C

Легковесная библиотека на C для вывода цветного и форматированного текста в терминал с использованием ANSI escape-кодов.

## Установка

### Сборка из исходников
```bash
git clone <url-репозитория>
cd msg_term_color
make
sudo make install

###  *** msg_term_color ***
Установка статической библиотеки libmsgtermcolor.a:

# Копируем заголовочный файл в системные include
sudo cp msg_term_color.h /usr/local/include/

# Копируем библиотеку в системные lib
sudo cp libmsgtermcolor.a /usr/local/lib/

# Обновляем кэш библиотек (для динамических, но хорошая привычка)
sudo ldconfig

2. Проверка установки:

# Проверяем, что файлы на месте
ls -la /usr/local/include/msg_term_color.h
ls -la /usr/local/lib/libmsgtermcolor.a

# Проверяем доступность для компилятора
gcc -lmsgtermcolor --verbose 2>&1 | grep -A5 "library path"

#Быстрый старт

#include <msg_term_color.h>
int main() {
    msg_term_color(31, "Красный текст\n", NUL);
    msg_term_color(32, "Зеленый текст\n", NUL);
    return 0;
}

#Компиляция:

gcc ваша_программа.c -lmsgtermcolor -o ваша_программа
 Возможности
 - Цветной вывод – цвета текста, фона, 256-цветная палитра
 - Стили текста – жирный, подчеркнутый, мигающий, инверсный
 - Подстановка формата – вставка чисел в строки через %d

 Простой API – две основные функции покрывают все сценарии
 Справочник API
msg_term_color(int color1, const char* text1, ...)
Выводит цветной/стилизованный текст. Последовательность должна заканчиваться NUL

Пример:
msg_term_color(31, "Ошибка: ", 33, "предупреждение", 0, NUL);
msg_term_color_buf(int first_var, ...)

Буферизует целые числа для подстановки %d в следующем вызове msg_term_color. Заканчивается BUF.
Пример:
msg_term_color_buf(10, 20, BUF);
msg_term_color(34, "Значения: %d и %d\n", NUL);

 Коды цветов и стилей в программе: complex_modules

 Примеры использования:

 msg_term_color(5001, "Текст цветом #1\n", NUL);   // Темно-красный
 msg_term_color(5050, "Текст цветом #50\n", NUL);  // Зеленоватый

Пример 1: Система логирования

void log_error(const char* message) {
    msg_term_color(1, 31, "[ОШИБКА] ", 0, message, NUL);
}

void log_success(const char* message) {
    msg_term_color(1, 32, "[УСПЕХ] ", 0, message, NUL);
}

void log_warning(const char* message) {
    msg_term_color(1, 33, "[ПРЕДУПР] ", 0, message, NUL);
}

Статическая библиотека:

gcc -c msg_term_color.c -o msg_term_color.o
ar rcs libmsgtermcolor.a msg_term_color.o

Динамическая библиотека:
gcc -shared -fPIC msg_term_color.c -o libmsgtermcolor.so
sudo cp libmsgtermcolor.so /usr/local/lib/
sudo ldconfig
 Структура проекта
msg_term_color/
├── msg_term_color.c      # Исходный код библиотеки
├── msg_term_color.h      # Заголовочный файл
├── Makefile              # Система сборки
├── libmsgtermcolor.a     # Статическая библиотека
└── README.md            # Эта документация

⚠️ Ограничения
Не потокобезопасна – используйте в однопоточных приложениях
Буфер фиксированного размера – 256 байт для форматирования
Только ANSI-терминалы – не работает в Windows CMD без эмуляции
Подстановка только %d – поддерживает только целые числа
Статические переменные – один буфер на всю программу

 Обновление библиотеки
# Получить обновления
git pull origin main

# Пересобрать и переустановить
make clean
make
sudo make install
 Удаление из системы
 sudo make uninstall

Или вручную:
sudo rm /usr/local/include/msg_term_color.h
sudo rm /usr/local/lib/libmsgtermcolor.a
sudo rm /usr/local/lib/libmsgtermcolor.so 2>/dev/null || true

❓ Частые вопросы
Q: Почему выводится мусор после текста?
A: Не забывайте завершать вызов msg_term_color терминатором NUL

Q: Библиотека не линкуется с -lmsgtermcolor
A: Убедитесь, что библиотека установлена в /usr/local/lib/ или укажите путь явно:
gcc program.c -L/usr/local/lib -lmsgtermcolor -o program

Q: Цвета не отображаются в моём терминале
A: Проверьте поддержку ANSI-кодов:
echo -e "\033[31mКрасный\033[0m \033[32mЗеленый\033[0m"

Q: Можно ли использовать в многопоточных программах?
A: Нет, библиотека не потокобезопасна. Для многопоточности нужно модифицировать код.

 Лицензия
MIT License. Подробнее в файле LICENSE.

 Вклад в проект
 Приветствуются pull requests:

 Форкните репозиторий
 Создайте ветку для новой функции
 Добавьте тесты
 Откройте pull request
 Сообщения об ошибках
 Используйте Issues на GitHub для:
 Сообщений об ошибках
 Запросов новых функций
 Вопросов по использованию
 Разработано для Linux/macOS терминалов с поддержкой ANSI escape-кодов.

# Полезные ссылки
 ANSI Escape Codes - полный справочник
 256-цветная палитра - визуализатор
 Документация по termios

# Пример полной программы c подключенной библиотекой.
#include <msg_term_color.h>
int main() {
    // Самый простой вызов без varargs
    msg_term_color(31, "Test", NUL);
    return 0;
}

Компиляция и запуск. Указать библиотеку:
gcc example.c -lmsgtermcolor -o example

# Пример полной программы c включенным файлом msg_term_color.c
#include "msg_term_color.h"
int main() {
    // Самый простой вызов без varargs
    msg_term_color(31, "Test", NUL);
    return 0;
}
Компиляция и запуск. Указать файл:
gcc test_mini.c msg_term_color.c -o test_mini

В случае глюка не срабатывания отключить буферизацию:
    setbuf(stdout, NULL);  // отключить буферизацию
    msg_term_color(37," END", NUL); // работает
