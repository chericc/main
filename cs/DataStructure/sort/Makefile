
CPP=g++
CFLAGS=-std=c++11
CFLAGS+=-g -Wall

TARGET=sort.out

SRC=$(wildcard *.cpp)
HEAD=$(wildcard *.h)

$(TARGET):$(SRC) $(HEAD)
	$(CPP) $(SRC) $(CFLAGS) -o $@

.PHONY:
	clean

clean:
	rm -rf $(TARGET)