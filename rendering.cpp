#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "helper_structs.h"
#include "rendering.h"
#include "main.h"

#include<iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>


#define 	GLFW_TRUE   1
#define		V_SYNC		true


static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"attribute vec2 vTex;\n"
"varying vec3 color;\n"
"varying vec2 tex;\n"
"void main()\n"
"{\n"
"	gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
// "	gl_Position = vec4(vPos, 0.0, 1.0);\n"
"	color = vCol;\n"
"	tex = vTex;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec3 color;\n"
"varying vec2 tex;\n"
"void main()\n"
"{\n"
"	float r = (tex.x - .5f) * (tex.x - .5f) + (tex.y - .5f) * (tex.y - .5f);"
"	float a = r < 0.25f;"
// "	a *= color.z > 0;"
// "	color *= (tex.x > .05) & (tex.x < .95);"
// "	color *= (tex.y > .05) & (tex.y < .95);"
"	gl_FragColor = vec4(color, a);\n"
"}\n";

void Rendering::draw_triangle(GLFWwindow* window, GLuint program, GLint mvp_location, mat4x4 m) {
	float ratio;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float) height;
	glViewport(0, 0, width, height);


	mat4x4 p, mvp;
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

	glDrawArrays(GL_TRIANGLES, 0, 3);

}

void Rendering::draw_line(GLFWwindow* window, GLuint program, GLint mvp_location) {

	float ratio;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float) height;
	glViewport(0, 0, width, height);

	mat4x4 m, p, mvp;
	mat4x4_identity(m);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	glDrawArrays(GL_LINES, 0, 2);
}


// void Rendering::draw_grid(GLFWwindow* window, GLuint program, GLint mvp_location, playfield field) {

// }

void Rendering::error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void Rendering::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void Rendering::key_poll(GLFWwindow* window) {
/*
	int w_state = glfwGetKey(window, GLFW_KEY_W);
	int a_state = glfwGetKey(window, GLFW_KEY_A);
	int s_state = glfwGetKey(window, GLFW_KEY_S);
	int d_state = glfwGetKey(window, GLFW_KEY_D);
*/
}


void Rendering::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetCursorPos(window, &xpos, &ypos);
		// printf("position: %.2f, %.2f\n", xpos, ypos);
		get_instance().main_script->create_tower(window, (float)xpos, (float)ypos);
	}
}




int Rendering::startGL(Main* script) {

	get_instance().main_script = script;

	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location, vtex_location;

	glfwSetErrorCallback(error_callback);

	/* Initialize the library */
	if (!glfwInit()) {
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	if (V_SYNC) {
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	} else {
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	}

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(Rendering::WIDTH, Rendering::HEIGHT, 
		"Tower Defence", NULL, NULL);
	
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval(1);
	// glfwSwapInterval(0);
	// NOTE: OpenGL error checks have been omitted for brevity
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	mvp_location = glGetUniformLocation(program, "MVP");
	vpos_location = glGetAttribLocation(program, "vPos");
	vcol_location = glGetAttribLocation(program, "vCol");
	vtex_location = glGetAttribLocation(program, "vTex");

	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
						  sizeof(vert), (void*) (sizeof(float) * 0));
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
						  sizeof(vert), (void*) (sizeof(float) * 2));
	glEnableVertexAttribArray(vtex_location);
	glVertexAttribPointer(vtex_location, 2, GL_FLOAT, GL_FALSE,
						  sizeof(vert), (void*) (sizeof(float) * 5));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double previous_time = glfwGetTime();
	int frame_count = 0;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {

		double current_time = glfwGetTime();
    	frame_count++;
    	frame_time = current_time - previous_time;
    	previous_time = current_time;

		glClear(GL_COLOR_BUFFER_BIT);

		main_script->update(window, program, mvp_location);

		if (V_SYNC) {
			glfwSwapBuffers(window);
		} else {
			glFlush();
		}

		glfwPollEvents();
		key_poll(window);
	}

	glfwTerminate();
	return 0;	
}