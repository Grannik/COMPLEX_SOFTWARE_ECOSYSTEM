# Компилятор
CC = gcc
CFLAGS = -Wall -Wextra -I.

# Имена файлов
LIB_NAME = libtermcolor
SRC = libtermcolor.c
OBJ = libtermcolor.o
HEADER = libtermcolor.h
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

# Быстрая проверка библиотеки
check: $(TARGET)
	@echo "=== Проверка символов в библиотеке ==="
	nm $(TARGET) | grep "T "
	@echo "=== Размер библиотеки ==="
	ls -lh $(TARGET)
