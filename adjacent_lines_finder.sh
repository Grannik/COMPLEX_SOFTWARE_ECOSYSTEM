#!/bin/sh

clear

if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage:"
    echo "  $0 <file> <pattern>"
    echo "  $0 <file> <pattern1> <pattern2>"
    exit 1
fi

FILE="$1"
PAT1="$2"
PAT2="$3"

if [ ! -f "$FILE" ]; then
    echo "Error: file not found: $FILE"
    exit 1
fi

cat -n "$FILE" | awk -v p1="$PAT1" -v p2="$PAT2" -v argc="$#" '

# ONE PATTERN MODE: print ALL matching lines
argc == 2 {
    if (index($0, p1) > 0) {
        print
    }
    next
}

# TWO PATTERN MODE: find first adjacent pair
argc == 3 {
    lines[NR] = $0
}

END {
    if (argc != 3) exit

    for (i = 1; i < NR; i++) {
        if (index(lines[i], p1) > 0 && index(lines[i+1], p2) > 0) {
            print lines[i]
            print lines[i+1]
            exit
        }
    }
}
'

#Возникла проблема
#terminalNavigatorRaw>./adjacent_lines_finder.sh terminalNavigatorRaw.c "if (write(STDOUT_FILENO, "\033[3J\033[2J\033[H", 11) < 0) {"
#terminalNavigatorRaw>./adjacent_lines_finder.sh terminalNavigatorRaw.c 'if (write(STDOUT_FILENO, "\033[3J\033[2J\033[H', 11) < 0) {"
#bash: syntax error near unexpected token `)'
#Надо её решить, так чтобы скрипт обрабатывал такие строки

# иногда на 1 ошибается
