CC = g++
LIBS = `pkg-config --cflags glfw3 glu` `pkg-config --static --libs glfw3`# `pkg-config --libs glu`
all:
	$(CC) main.cpp game_classes.cpp rendering.cpp glad.c -O2 -g -o main.out -std=c++11 $(LIBS)