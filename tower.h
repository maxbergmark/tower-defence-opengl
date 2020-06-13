#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "helper_structs.h"
#include "game_classes.h"


class Tower : public GameObject {

private:
	Tower(float2 center, float2 size, GameGrid* grid);
	void configure(float2 center, float2 size, GameGrid* grid);

public:
	quad q;
	float speed;
	float damage;
	float range;
	int frame;
	int2 cell;
	GameGrid* game_grid;
	Enemy* current_target;
	int pool_idx;

	static int pool_size;
	static int active_towers;
	static std::vector<Tower*> tower_pool;
	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	bool out_of_range();
	void find_new_target();

	static void create_tower_pool(int n, GameGrid* grid);
	static void update_towers();
	static void late_update_towers();
	static void draw_towers(GLFWwindow* window, GLuint program, GLint mvp_location);
	static bool initialized;
	static Tower* add_tower(float2 center, float2 size, GameGrid* grid);
	static void remove_tower(Tower* t);

};