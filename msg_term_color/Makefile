# Компилятор
CC = gcc
CFLAGS = -Wall -Wextra -I.

# Имена файлов
LIB_NAME = libmsgtermcolor
SRC = msg_term_color.c
OBJ = msg_term_color.o
HEADER = msg_term_color.h
TARGET = $(LIB_NAME).a

# Правила по умолчанию
.PHONY: all clean install uninstall

all: $(TARGET)

# Создание объектного файла
$(OBJ): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание статической библиотеки
$(TARGET): $(OBJ)
	ar rcs $@ $^

# Очистка
clean:
	rm -f $(OBJ) $(TARGET)

# Установка в систему (требует прав sudo)
install: $(TARGET)
	cp $(HEADER) /usr/local/include/
	cp $(TARGET) /usr/local/lib/
	ldconfig

# Удаление из системы
uninstall:
	rm -f /usr/local/include/$(HEADER)
	rm -f /usr/local/lib/$(TARGET)
	ldconfig

# Тестовая программа
test: test.c $(TARGET)
	$(CC) $(CFLAGS) test.c -L. -lmsgtermcolor -o test

# Быстрая проверка библиотеки
check: $(TARGET)
	@echo "=== Проверка символов в библиотеке ==="
	nm $(TARGET) | grep "T "
	@echo "=== Размер библиотеки ==="
	ls -lh $(TARGET)
