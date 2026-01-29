
# COMPLEX_SOFTWARE_ECOSYSTEM
Проект для отображения разных модулей.

## Установка
Используйте Makefile для упрощения: make
Это соберет исполняемый файл `complex_modules`.

## Сборка отдельных модулей
В последней строке каждого модуля

## Запуск
Запустите приложение: ./complex_modules

Программа стартует с модуля 0. Для запуска конкретного модуля используйте цели Makefile, такие как make run0, make run1 и т.д.

## Управление
- Цифры 0-7: Переключение между модулями.
- q: Выход из программы.

## Описание модулей
В нулевом модуле

## Структура файлов
- `common.h` / `common.c`: Утилиты для работы с терминалом (режимы, отрисовка).
- `complex_modules.c`: Основной код для переключения между модулями.
- `module_00.c` – `module_07.c`: Реализации модулей.
- `pseudographic_font_3.h`, `pseudographic_font_5.h`, `pseudographic_font_7.h`: Шрифты для псевдографики.
- `content_help.h` Справка
- `content_var.c` `content_var.h` Контент переменных.
- `Makefile`: Скрипт для сборки и запуска.
- `README.md`: Это руководство.

## Требования
- POSIX-совместимая система (Linux, macOS, WSL).
- Компилятор GCC.
- Терминал с поддержкой ANSI escape-кодов и UTF-8 (для корректного отображения символов).

## Работа
❌ НЕТ фоновой работы модулей
❌ НЕТ многопоточности или параллельного выполнения
✅ Каждый модуль работает последовательно, полностью завершаясь перед запуском следующего
✅ Переключение модуля = остановка предыдущего + запуск нового с чистого экрана
#=====================================================================================

# libtermcolor – Библиотека цветного вывода в терминал на C
Легковесная библиотека на C для вывода цветного и форматированного текста в терминал с использованием ANSI escape-кодов.

## Установка

### Сборка из исходников
```bash
git clone <url-репозитория>
cd libmsgtermcolor
make
sudo make install

###  *** libmsgtermcolor ***
#Установка статической библиотеки libmsgtermcolor.a:

 Копируем заголовочный файл в системные include:
sudo cp libtermcolor.h /usr/local/include/

 Копируем библиотеку в системные lib
sudo cp libtermcolor.a /usr/local/lib/

 Обновляем кэш библиотек (для динамических, но хорошая привычка)
sudo ldconfig

2. Проверка установки:

# Проверяем, что файлы на месте
ls -la /usr/local/include/libtermcolor.h
ls -la /usr/local/lib/libtermcolor.a

# Проверяем доступность для компилятора
gcc -lmsgtermcolor --verbose 2>&1 | grep -A5 "library path"

#Компиляция:
gcc ваша_программа.c -lmsgtermcolor -o ваша_программа
 Возможности
 - Цветной вывод – цвета текста, фона, 256-цветная палитра
 - Стили текста – жирный, подчеркнутый, мигающий, инверсный
 - Подстановка формата – вставка чисел в строки через %d

# Статическая библиотека:

gcc -c msg_term_color.c -o msg_term_color.o
ar rcs libmsgtermcolor.a msg_term_color.o

# Динамическая библиотека:
gcc -shared -fPIC msg_term_color.c -o libmsgtermcolor.so
sudo cp libmsgtermcolor.so /usr/local/lib/
sudo ldconfig

# ⚠️  Ограничения
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
 Удаление из системы: sudo make uninstall

Или вручную:
sudo rm /usr/local/include/libmsgtermcolor.h
sudo rm /usr/local/lib/libmsgtermcolor.a

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

# Полезные ссылки
 ANSI Escape Codes - полный справочник
 256-цветная палитра - визуализатор
 Документация по termios

# Пример полной программы c подключенной библиотекой.
#include <libtermcolor.h>
int main() {
    libtermcolor_msg(31, "Test", NUL);
    return 0;
}
 Компиляция и запуск. Указать библиотеку: gcc example.c -lmsgtermcolor -o example

# Пример полной программы c включенным файлом msg_term_color.c
#include "libtermcolor.h"
int main() {
    libtermcolor_msg(31, "Test", NUL);
    return 0;
}
Компиляция и запуск. Указать файл: gcc example.c libmsgtermcolor.c -o example

# === Структура проекта ==================================================

COMPLEX_SOFTWARE_ECOSYSTEM
├── common.c
├── common.h
├── complex_modules.c
├── content_color_16.h
├── content_color_3_256.h
├── content_color_4_256.h
├── content_help.h
├── content_var.c
├── content_var.h
├── libtermcolor
│   ├── libtermcolor.c
│   ├── libtermcolor.h
│   └── Makefile
├── Makefile
├── module_00.c
├── module_01.c
├── module_02.c
├── module_03.c
├── module_04.c
├── module_05.c
├── module_06.c
├── module_07.c
├── module_08.c
├── pseudographic_font_3.h
├── pseudographic_font_5.h
└── pseudographic_font_7.h

## Публикация
 neocities:   https://grannik.neocities.org/grCterm
 codeberg:    https://codeberg.org/Grannik/COMPLEX_SOFTWARE_ECOSYSTEM
 github:      https://github.com/Grannik/COMPLEX_SOFTWARE_ECOSYSTEM
 gitlab:      https://gitlab.com/grannik/complex_software_ecosystem
 sourceforge: https://sourceforge.net/projects/complex-software-ecosystem/
