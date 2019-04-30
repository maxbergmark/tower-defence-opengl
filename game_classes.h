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

	GameObject() {}
	virtual void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const {
		printf("wrong draw\n");
	};
};


class Tower : public GameObject {
public:
	quad q;
	float speed;
	float damage;
	float range;
	int frame;
	int2 cell;
	GameGrid* game_grid;
	Enemy* current_target;

	Tower(float2 center, float2 size, GameGrid* grid);
	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	bool out_of_range();
	void find_new_target();
};

class Enemy : public GameObject {
private:
	Enemy(float2 center, float2 size, GameGrid* grid);
	void configure(float2 center, float2 size, GameGrid* grid);
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
	GameGrid* game_grid;
	static std::vector<Enemy*> enemy_pool;
	static void create_enemy_pool(int n);
	static bool initialized;
	static Enemy* get_enemy(float2 center, float2 size, GameGrid* grid);
	static void return_enemy(Enemy* e);

	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	void apply_damage(float damage);
};

class GameGrid : public GameObject {
public:
	// std::vector<Tower> towers;
	std::vector<std::unordered_set<Enemy*>*> enemies;
	Main* main_script;
	playfield field;
	GameGrid(int x, int y, Main* script);
	void update();
	void late_update();
	void draw(GLFWwindow* window, GLuint program, GLint mvp_location) const;

	void calculate_directions();
	void find_cell(float xpos, float ypos,
		float2 &center, float2 &size, bool &should_place);
	void get_direction(float xpos, float ypos, float &x_dir, float &y_dir, bool &at_goal);
	void get_cell(float2 pos, int2 &cell);
	void create_tower(GLFWwindow* window, float xpos, float ypos, std::vector<GameObject*> &game_objects);
	void remove_enemy(Enemy* e);
	std::unordered_set<Enemy*>* get_enemies(int2 cell);
	void change_cell(Enemy* e);
};
