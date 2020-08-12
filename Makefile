CC::=gcc -std=gnu11 -Wno-unused-parameter -Wall -Wextra -O0 -g

00_restful.out:00_restful.c
	$(CC) $(curl-config --cflags) -o $@ $< $(shell curl-config --libs)
