#pragma once

#include <vector>
#include "game_classes.h"
#include "helper_structs.h"
#include <utility>
#include <memory>

class Rendering;

class Main {

	public:
		std::vector<GameObject*> game_objects;
		GameGrid* game_grid;
		GameObject* object_buffer;
		float* object_depth_buffer;
		long frame;
		// std::unique_ptr<Rendering> r;
		Rendering* r;

		Main();
		void start();
		void create_gamegrid();
		void create_tower(GLFWwindow* window, float xpos, float ypos);
		void update(GLFWwindow* window, GLuint program, GLint mvp_location);

};

class Time {
	private: 
	public:
		static double last_frame;
		static double delta;
		static double time;
		static double runtime;

		static void update_time();
};