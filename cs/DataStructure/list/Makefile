# Makefile for list

GCC=gcc
SRC=list.c main.c
TARGET=list.out

list: $(SRC)
	$(GCC) $(SRC) -o $(TARGET) -g -Wall

all: list

clean:
	rm -v $(TARGET)

.PHONY: list all clean