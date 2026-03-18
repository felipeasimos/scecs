CC = cc
CC_FLAGS = -std=c99 -Wall -lraylib -lm -lX11 -I include/ -l:libraylib.a

all: debug

debug:
	$(CC) main.c -o main $(CC_FLAGS) -O0 -g -fsanitize=address && ./main
valgrind:
	# $(CC) main.c -o main $(CC_FLAGS) -O0 -g -fsanitize=address
	$(CC) main.c -o main $(CC_FLAGS) -O0 -g && valgrind --leak-check=full --exit-on-first-error=yes --error-exitcode=1 --quiet ./main

release:
	$(CC) main.c -o main $(CC_FLAGS) -O3
