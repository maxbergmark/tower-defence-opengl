CC = g++
# LIBS = `pkg-config --cflags glfw3 glu` `pkg-config --static --libs glfw3`# `pkg-config --libs glu`
LIBS = `pkg-config --cflags glfw3` `pkg-config --static --libs glfw3`# `pkg-config --libs glu`
all:
	$(CC) main.cpp helper_structs.cpp game_classes.cpp tower.cpp enemy.cpp rendering.cpp glad.c -O2 -g -o main.out -std=c++11 $(LIBS) -Wall