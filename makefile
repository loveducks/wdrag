all: pong

pong: pong.c
	gcc -g -fsanitize=address pong.c -lncurses -lmenu -o out.o
