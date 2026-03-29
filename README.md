# COMPLEX SOFTWARE ECOSYSTEM

Многофункциональное терминальное приложение, демонстрирующее различные возможности ANSI и ncurses, включая множество вариантов отображения часов, просмотр цветовых палитр и интерактивные модули с прокруткой контента.

## 🚀 Возможности

- **Двухрежимная работа**: плавное переключение между режимами ncurses и чистым ANSI
- **Множество вариантов часов**:
  - Стандартный формат в одну строку
  - 3-строчный псевдографический шрифт
  - 5-строчный псевдографический шрифт
  - 7-строчный псевдографический шрифт
- **Просмотр цветовых палитр**:
  - Базовые 16-цветные атрибуты ANSI
  - 256-цветная палитра переднего плана
  - 256-цветная палитра фона
- **Интерактивная прокрутка** в модулях с контентом
- **Навигация между модулями** с помощью клавиш 0-9 и a-z
- **Проверка размера терминала** (минимум 84x21)
- **Поддержка Unicode и широких символов**

## 📦 Структура проекта

COMPLEX_SOFTWARE_ECOSYSTEM
├── common.c
├── common.h
├── complex_modules.c
├── content_color
│   ├── content_abc.h
│   ├── content_color_16.h
│   ├── content_color_3_256.h
│   └── content_color_4_256.h
├── content_var
│   ├── content_var.c
│   └── content_var.h
├── file_analyzer
│   ├── analyzer_wrapper.c
│   ├── analyzer_wrapper.h
│   └── file_analyzer.c
├── libtermanimation
│   ├── libtermanimation.c
│   └── libtermanimation.h
├── libtermcolor
│   ├── libtermcolor.c
│   ├── libtermcolor.h
│   ├── Makefile
│   ├── pseudographic.c
│   ├── pseudographic_font_3.h
│   ├── pseudographic_font_5.h
│   ├── pseudographic_font_7.h
│   ├── pseudographic_font_9.h
│   ├── pseudographic.h
│   ├── tc_file_directories_color.c
│   ├── tc_file_directories_color.h
│   ├── tc_file_types.h
│   ├── tc_frame.c
│   └── tc_frame.h
├── libtermcontrol
│   ├── libtermcontrol.c
│   └── libtermcontrol.h
├── Makefile
├── module00
│   ├── content_help.h
│   └── module_00.c
├── module01
│   ├── module_01.c
│   └── module_01_standalone.c
├── module_02.c
├── module_03.c
├── module_04.c
├── module_05.c
├── module_06.c
├── module_07.c
├── module_08.c
├── module_09.c
├── module_10.c
├── module_11.c
└── module_12.c

## 🛠️ Сборка и запуск

```bash
# Клонирование репозитория
git clone https://github.com/yourusername/COMPLEX_SOFTWARE_ECOSYSTEM.git
cd COMPLEX_SOFTWARE_ECOSYSTEM

# Сборка проекта
make

# Запуск
./complex_modules

 Управление
0-9, a-z - переключение между модулями
Q - выход из программы
Стрелки вверх/вниз - прокрутка контента (в соответствующих модулях)

 Требования
Терминал с поддержкой 256 цветов (рекомендуется)

Минимальный размер терминала: 84x21 символов

Unix-подобная ОС (Linux, macOS, *BSD)

Библиотека ncurses

 Лицензия
MIT License

 Автор: Grannik

 Github: https://github.com/Grannik/COMPLEX_SOFTWARE_ECOSYSTEM
