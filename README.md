# COMPLEX SOFTWARE ECOSYSTEM

Многофункциональное терминальное приложение, демонстрирующее различные возможности ANSI и ncurses, включая множество вариантов отображения часов, просмотр цветовых палитр и интерактивные модули с прокруткой контента.

## 🚀 Возможности

- **Двухрежимная работа**: плавное переключение между режимами ncurses и чистым ANSI
- **Множество вариантов часов**:
  - Стандартный формат в одну строку
  - 3-строчный псевдографический шрифт
  - 5-строчный псевдографический шрифт
  - 7-строчный псевдографический шрифт
  - 9-строчный псевдографический шрифт

- **Просмотр цветовых палитр**:
  - Базовые 16-цветные атрибуты ANSI
  - 256-цветная палитра переднего плана
  - 256-цветная палитра фона
- **Интерактивная прокрутка** в модулях с контентом
- **Навигация между модулями** с помощью клавиш 0-9 и a-z
- **Проверка размера терминала** (минимум 84x21)
- **Поддержка Unicode и широких символов**

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

# Audio Player Pcm

**GRANNIK Terminal Navigator Audio Player Pcm** — терминал-базированный (TTY) файловый навигатор и плеер сырых PCM-аудиофайлов (`.raw`) формата **44100 Hz / 16-bit / stereo**.
Часть большой экосистемы **GRANNIK COMPLEX SOFTWARE ECOSYSTEM** — десятки утилит на чистом C, работающих только под Linux + GNU libc.

## Особенности
- Навигация по директориям с сортировкой (папки сверху, алфавитно)
- Воспроизведение RAW PCM без заголовков (44100/16/2 — фиксированный формат)
- Плейлисты: загрузка всех `.raw`-файлов из выбранной папки
- Перемотка ±10 сек, пауза, стоп
- История навигации (вперёд/назад)
- Прогресс-бар, текущее время, системное время
- Полностью многопоточный плеер (pthread + ALSA)
- UTF-8, цветной интерфейс на ncursesw
- Защита от symlink-атак (O_NOFOLLOW), обработка всех ошибок

## Требования
- Linux + GNU libc
- ncursesw
- libasound2 (ALSA)
- pthread (входит в glibc)

Key Action
↑ ↓   Move through the list
←     Go back through history
→     Jump forward in history
Enter Войти в папку или воспроизвести .raw
[space] play directory as playlist
-       seek backward
=       seek forward
[       scroll file path left
]       scroll file path right
;       loop playback
,       pause playback
.       stop playback
/       play previous file
\       play next file

Формат аудио: Только сырой PCM:
44100 Hz 16-bit little-endian 2 канала (стерео) Никаких заголовков — прямой поток байт.

FILE REQUIREMENTS: | ТРЕБОВАНИЯ К ФАЙЛАМ:

 Only .raw files are played, .raw PCM without header
 Воспроизводятся только файлы .raw, PCM без заголовка

 s16le, 2 channels, 44100 Hz, 16 bits/sample, stereo, little-endian
 s16le, 2 канала, 44100 Гц, 16 бит на сэмпл, стерео, порядок байтов little-endian

 Пример команды конвертации:
 ffmpeg -i input.mp3 -f s16le -ac 2 -ar 44100 output.raw

Требования: Терминал с поддержкой 256 цветов (рекомендуется)
Минимальный размер терминала: 84x21 символов
Unix-подобная ОС (Linux, macOS, *BSD)
Библиотека ncurses
License: MIT License
Author: GRANNIK

 neocities: https://grannik.neocities.org/grCterm
 Github:   https://github.com/Grannik/COMPLEX_SOFTWARE_ECOSYSTEM
 Codeberg: https://codeberg.org/Grannik/COMPLEX_SOFTWARE_ECOSYSTEM
