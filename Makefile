# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -DCOMPLEX_BUILD
LDFLAGS =

# Target executable
TARGET = complex_modules

# Source files
SRCS = common.c complex_modules.c \
	module_00.c module_01.c module_02.c module_03.c module_04.c module_05.c module_06.c

# Object files
OBJS = $(SRCS:.c=.o)

# Header files
HEADERS = common.h pseudographic_font_3.h pseudographic_font_5.h pseudographic_font_7.h pseudographic_help.h pseudographic_color.h

# Default target
all: $(TARGET)

# Build main executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Object file dependencies
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

complex_modules.o: complex_modules.c common.h
	$(CC) $(CFLAGS) -c complex_modules.c -o complex_modules.o

module_00.o: module_00.c common.h pseudographic_help.h
	$(CC) $(CFLAGS) -c module_00.c -o module_00.o

module_01.o: module_01.c common.h pseudographic_font_3.h
	$(CC) $(CFLAGS) -c module_01.c -o module_01.o

module_02.o: module_02.c common.h pseudographic_font_5.h
	$(CC) $(CFLAGS) -c module_02.c -o module_02.o

module_03.o: module_03.c common.h pseudographic_font_7.h
	$(CC) $(CFLAGS) -c module_03.c -o module_03.o

module_04.o: module_04.c common.h
	$(CC) $(CFLAGS) -c module_04.c -o module_04.o

module_05.o: module_05.c common.h
	$(CC) $(CFLAGS) -c module_05.c -o module_05.o

module_06.o: module_06.c common.h
	$(CC) $(CFLAGS) -c module_06.c -o module_06.o

# Standalone executables (optional)
standalone: module_00_standalone module_01_standalone module_02_standalone module_03_standalone module_04_standalone module_05_standalone module_06_standalone

module_00_standalone: module_00.c common.c common.h pseudographic_help.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_00 module_00.c common.c

module_01_standalone: module_01.c common.c common.h pseudographic_font_3.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_01 module_01.c common.c

module_02_standalone: module_02.c common.c common.h pseudographic_font_5.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_02 module_02.c common.c

module_03_standalone: module_03.c common.c common.h pseudographic_font_7.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_03 module_03.c common.c

module_04_standalone: module_04.c common.c common.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_04 module_04.c common.c

module_05_standalone: module_05.c common.c common.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_05 module_05.c common.c

module_06_standalone: module_06.c common.c common.h
	$(CC) -Wall -Wextra -O2 -std=c99 -o module_06 module_06.c common.c

# Clean build files
clean:
	rm -f $(TARGET) $(OBJS) \
	      module_00 module_01 module_02 module_03 module_04 module_05 module_06

# Run the program (start with module 0 by default)
run: $(TARGET)
	./$(TARGET)

# Run with specific module
run0: $(TARGET)
	./$(TARGET) 0
run1: $(TARGET)
	./$(TARGET) 1
run2: $(TARGET)
	./$(TARGET) 2
run3: $(TARGET)
	./$(TARGET) 3
run4: $(TARGET)
	./$(TARGET) 4
run5: $(TARGET)
	./$(TARGET) 5
run6: $(TARGET)
	./$(TARGET) 6

# Show help
help: $(TARGET)
	./$(TARGET) mh

# Phony targets
.PHONY: all clean run run0 run1 run2 run3 run4 run5 run6 help standalone
