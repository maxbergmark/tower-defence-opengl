#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "helper_structs.h"




class GameGrid;
class Main;
class Tower;
class Enemy;

class GameObject {
public:
	virtual void start() {}
	virtual void update() {}
	virtual void late_update() {}
	std::string name;
	float2 position;
	float2 size;

	GameObject() {}
	virtual void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const {
		printf("wrong draw\n");
	};
};


class GameGrid : public GameObject {
public:
	std::vector<Tower*> towers;
	std::vector<std::unordered_set<Enemy*>*> enemies;
	Main* main_script;
	playfield field;
	GameGrid(int x, int y, Main* script);
	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	void calculate_directions();
	Tower* find_tower(int2 cell);
	int2 find_cell(float xpos, float ypos,
		float2 &center, float2 &size, bool &should_place);
	void get_direction(float xpos, float ypos, float &x_dir, float &y_dir, bool &at_goal);
	void get_cell(float2 pos, int2 &cell);
	void create_tower(GLFWwindow* window, float xpos, float ypos, std::vector<GameObject*> &game_objects);
	void remove_enemy(Enemy* e);
	std::unordered_set<Enemy*>* get_enemies(int2 cell);
	void change_cell(Enemy* e);
};
