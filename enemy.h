#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "helper_structs.h"
#include "game_classes.h"

class Enemy : public GameObject {

private:
	Enemy(float2 center, float2 size, GameGrid* grid);
	void configure(float2 center, float2 size, GameGrid* grid);
	void align_verts() const;

public:
	quad q;
	float2 velocity;
	int2 cell;
	int2 last_cell;
	float max_speed;
	float damage;
	float health;
	float max_health;
	bool alive;
	int pool_idx;
	static GameGrid* game_grid;
	static int pool_size;
	static int active_enemies;
	static std::vector<Enemy*> enemy_pool;
	static std::vector<quad> enemy_quads;

	static void create_enemy_pool(int n, GameGrid* grid);
	static void update_enemies();
	static void late_update_enemies();
	static void draw_enemies(GLFWwindow* window, GLuint program, GLint mvp_location);
	static bool initialized;
	static void add_enemy(float2 center, float2 size, GameGrid* grid);
	static void remove_enemy(Enemy* e);

	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	void apply_damage(float damage);
};