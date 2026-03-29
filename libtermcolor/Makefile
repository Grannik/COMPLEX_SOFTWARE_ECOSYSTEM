# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC

# Имя библиотеки
LIB_NAME = libtermcolor.a

# Исходники библиотеки
SRC = libtermcolor.c \
      tc_frame.c \
      tc_file_directories_color.c \
      pseudographic.c

# Объектные файлы
OBJ = $(SRC:.c=.o)

# =========================

# Главная цель: сборка библиотеки
all: $(LIB_NAME)

# Сборка статической библиотеки
$(LIB_NAME): $(OBJ)
	ar rcs $@ $^

# Компиляция .c → .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f *.o $(LIB_NAME)

# Полная пересборка
re: clean all
