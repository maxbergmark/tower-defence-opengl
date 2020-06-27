CC = g++
SRC_DIR := src
BIN_DIR := bin
SRC = $(wildcard $(SRC_DIR)/*.cpp)
# OBJ = $(SRC:.cpp=.o)
BIN = ./bin
TARGET = $(BIN)/tower_defence

LIBS = `pkg-config --cflags glfw3` `pkg-config --static --libs glfw3`# `pkg-config --libs glu`
CFLAGS = -I./include -g -O3 -Wall -std=c++11

all:
	g++ $(SRC) -o $(TARGET) -std=c++11 $(LIBS) $(CFLAGS)


dir: $(BIN)

$(BIN):
	mkdir -p $(BIN)

%.o: %.cpp
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

$(BIN)/$(TARGET): $(OBJ)
	$(CC) -o $@ $^


clean:
	rm -f $(OBJ) $(BIN)/$(TARGET)
