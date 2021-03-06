#include <sys/time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "linmath.h"
#include "game_classes.h"
#include "helper_structs.h"
#include "rendering.h"
#include "main.h"

double Time::time;
double Time::delta;
double Time::last_frame;
double Time::runtime;

int main(void) {
	Main script;
	script.start();
}

Main::Main() {
	frame = 0;
	r = &Rendering::get_instance();
}

void Main::start() {
	this->create_gamegrid();
	Time::last_frame = 0;
	game_objects.reserve(1<<16);
	r->startGL(this);
}

void Main::create_gamegrid() {
	game_grid = new GameGrid(21, 22, this);
	// game_grid = new GameGrid(41, 42, this);
	game_objects.push_back(game_grid);
}

void Main::create_tower(GLFWwindow* window, float xpos, float ypos) {
	game_grid->create_tower(window, xpos, ypos, game_objects);
}

void Main::update(GLFWwindow* window, GLuint program, GLint mvp_location) {

	Time::update_time();
	// printf("1\n");
	// printf("count: %d/%d\n", game_objects.size(), game_objects.capacity());
	struct timeval t0, t1, t2;

	gettimeofday(&t0, NULL);
	for (auto g : game_objects) {
		// std::cout << g << "\t" << std::endl;
		// printf("name: %s", g->name);
		g->update();
	}
	// printf("2\n");

	for (auto g : game_objects) {
		g->late_update();
	}
	gettimeofday(&t1, NULL);
	// printf("update time: %.3fms\n", (t1.tv_usec - t0.tv_usec) * 1e-3);
	// printf("3\n");

	for (auto g : game_objects) {
		g->draw(window, program, mvp_location);
	}
	gettimeofday(&t2, NULL);
	// printf("draw time: %.3fms\n", (t2.tv_usec - t1.tv_usec) * 1e-3);
	// printf("4\n");
	frame++;
}

void Time::update_time() {

	struct timeval start;
	long seconds, useconds;

	gettimeofday(&start, NULL);
	seconds = start.tv_sec;
	useconds = start.tv_usec;
	if (Time::last_frame == 0) {
		Time::delta = 0;
	} else {
		Time::delta = seconds + useconds * 1e-6 - Time::last_frame;
	}
	Time::time = seconds + useconds * 1e-6;
	Time::runtime += Time::delta;
	Time::last_frame = seconds + useconds * 1e-6;

}