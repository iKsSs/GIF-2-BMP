PROG_NAME := gif2bmp
SOURCE := main.c gif2bmp.c
CC := gcc
CFLAGS += -std=c11 -Wall -Wextra -pedantic

DIR := kko.proj3.xpastu00

DEBUG_NAME := gif2bmp_debug

.PHONY: all clean

all: $(PROG_NAME)

rebuild: clean all

$(PROG_NAME):
	$(CC) $(CFLAGS) $(SOURCE) -o $@

debug:
	$(CC) -ggdb3 $(SOURCE) -o $(DEBUG_NAME)

pack:
	mkdir -p ./$(DIR)
	cp -t ./$(DIR) main.c gif2bmp.c gif2bmp.h Makefile gif2bmp.pdf
	zip -r kko\.proj3\.xpastu00.zip kko\.proj3\.xpastu00

clean:
	rm -f *~ *.o $(PROG_NAME)
