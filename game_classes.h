#pragma once

#include "helper_structs.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

class GameGrid;
class Enemy;
class Main;

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
