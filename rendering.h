#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>

#include "linmath.h"
#include "helper_structs.h"
#include "main.h"

#define GLFW_TRUE   1

class Rendering {

	float z_rotate, y_rotate;
	float frame_time;
	static const int WIDTH = 1920;
	static const int HEIGHT = 1080;
	Main* main_script;
	
public:
	static Rendering& get_instance() {
		static Rendering instance; // lazy singleton, instantiated on first use
		return instance;
	}

	void draw_triangle(GLFWwindow* window, GLuint program, GLint mvp_location, mat4x4 m);
	void draw_line(GLFWwindow* window, GLuint program, GLint mvp_location);
	// void draw_grid(GLFWwindow* window, GLuint program, GLint mvp_location, playfield field);
	static void error_callback(int error, const char* description);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void key_poll(GLFWwindow* window);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	// void find_cell(GLFWwindow* window, float xpos, float ypos,
		// float &x_left, float &x_right, float &y_top, float &y_bottom);
	void draw_quads(GLFWwindow* window, GLuint program, GLint mvp_location);
	// public:
	int startGL(Main* script);

};
