CC = gcc

CSTD = -std=c11

WARNINGS = -Wall -Wextra -Wpedantic -Werror \
-Wshadow -Wconversion -Wsign-conversion \
-Wstrict-prototypes -Wmissing-prototypes \
-Wold-style-definition -Wcast-qual \
-Wwrite-strings -Wformat=2 -Wnull-dereference \
-Wdouble-promotion

SANITIZERS = -fsanitize=address,undefined

DEPFLAGS = -MMD -MP

INCLUDES = -I. -Ilibtermcolor -Ilibanimation -Ilibtermcontrol

CFLAGS = $(CSTD) $(WARNINGS) $(SANITIZERS) $(DEPFLAGS) $(INCLUDES) $(PKG_CFLAGS) -g

LIBS = -lncursesw -lmagic -luchardet -larchive -lavformat -lavcodec -lavutil -lexif

PKG_CFLAGS := $(shell pkg-config --cflags libmagic uchardet libarchive libexif libavformat libavutil 2>/dev/null || echo "")

TARGET = complex_software

SRCS = \
complex_modules.c \
common.c \
libtermanimation/libtermanimation.c \
libtermcolor/libtermcolor.c \
libtermcolor/pseudographic.c \
libtermcolor/tc_frame.c \
libtermcolor/tc_file_directories_color.c \
libtermcontrol/libtermcontrol.c \
content_var/content_var.c \
file_analyzer/analyzer_wrapper.c \
file_analyzer/file_analyzer.c \
module00/module_00.c \
module01/module_01.c \
module_02.c \
module_03.c \
module_04.c \
module_05.c \
module_06.c \
module_07.c \
module_08.c \
module_09.c \
module_10.c \
module_11.c \
module_12.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)

.PHONY: clean

-include $(DEPS)
