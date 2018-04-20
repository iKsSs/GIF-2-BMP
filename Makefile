PROG_NAME := gif2bmp
SOURCE := main.c gif2bmp.c
CC := gcc
CFLAGS += -std=c11 -Wall -Wextra -pedantic

DIR := kko.proj3.xpastu00

DEBUG_NAME := gif2bmp_debug

.PHONY: all clean test

all: $(PROG_NAME)

rebuild: clean all

$(PROG_NAME):
	$(CC) $(CFLAGS) $(SOURCE) -o $@ -lm

debug:
	$(CC) -ggdb3 $(SOURCE) -o $(DEBUG_NAME) -lm

test:
	./$(PROG_NAME) -i ./test/easy.gif -o test.bmp 

test2:
	./$(PROG_NAME) -i ./test/ubuntu.gif -o test2.bmp 

testAll:
	./test.sh
	
pack:
	mkdir -p ./$(DIR)
	cp -t ./$(DIR) main.c gif2bmp.c gif2bmp.h Makefile gif2bmp.pdf
	zip -r kko\.proj3\.xpastu00.zip kko\.proj3\.xpastu00

clean:
	rm -f *~ *.o $(PROG_NAME)
