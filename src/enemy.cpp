#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_set>
#include <queue>
#include <bits/stdc++.h> 

#include "linmath.h"
#include "enemy.h"
#include "rendering.h"
#include "helper_structs.h"
#include "game_classes.h"


std::vector<Enemy*> Enemy::enemy_pool;
std::vector<quad> Enemy::enemy_quads;
int Enemy::pool_size = 0;
int Enemy::active_enemies = 0;
GameGrid* Enemy::game_grid;

class GameGrid;


Enemy::Enemy(float2 center, float2 size, GameGrid* grid) {
	configure(center, size, grid);
}

void Enemy::align_verts() const {

	for (int i = 0; i < 3; i++) {
		q.t0.verts[i].r = 1 - health / max_health;
		q.t0.verts[i].g = health / max_health;
		q.t0.verts[i].b = 0.f;

		q.t1.verts[i].r = 1 - health / max_health;
		q.t1.verts[i].g = health / max_health;
		q.t1.verts[i].b = 0.f;
	}
	// printf("size: %.3f, %.3f\n", size.x, size.y);
	float2 p = position;
	q.t0.verts[0].x = 1.0 * p.x - .5f * size.x;
	q.t0.verts[0].y = 1.0 * p.y + .5f * size.y;
	q.t0.verts[1].x = 1.0 * p.x + .5f * size.x;
	q.t0.verts[1].y = 1.0 * p.y - .5f * size.y;
	q.t0.verts[2].x = 1.0 * p.x - .5f * size.x;
	q.t0.verts[2].y = 1.0 * p.y - .5f * size.y;

	q.t1.verts[0].x = 1.0 * p.x + .5f * size.x;
	q.t1.verts[0].y = 1.0 * p.y - .5f * size.y;
	q.t1.verts[1].x = 1.0 * p.x + .5f * size.x;
	q.t1.verts[1].y = 1.0 * p.y + .5f * size.y;
	q.t1.verts[2].x = 1.0 * p.x - .5f * size.x;
	q.t1.verts[2].y = 1.0 * p.y + .5f * size.y;




	// float2 p = position;
	// q.t0.verts[0] = {0*p.x - .5f * size.x, 0*p.y + .5f * size.y, 0.f, 1.f, 0.f};
	// q.t0.verts[1] = {0*p.x + .5f * size.x, 0*p.y - .5f * size.y, 0.f, 1.f, 0.f};
	// q.t0.verts[2] = {0*p.x - .5f * size.x, 0*p.y - .5f * size.y, 0.f, 1.f, 0.f};

	// q.t1.verts[0] = {0*p.x + .5f * size.x, 0*p.y - .5f * size.y, 0.f, 1.f, 0.f};
	// q.t1.verts[1] = {0*p.x + .5f * size.x, 0*p.y + .5f * size.y, 0.f, 1.f, 0.f};
	// q.t1.verts[2] = {0*p.x - .5f * size.x, 0*p.y + .5f * size.y, 0.f, 1.f, 0.f};	
}

void Enemy::configure(float2 center, float2 sz, GameGrid* grid) {

	position = center;// - grid->field.world_center;
	size = sz;
	float vx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float vy = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	velocity = float2(vx - .5f, vy - .5f);


	// game_grid = grid;
	float ms = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	max_speed = 0.5f + ms / 5.f;
	max_health = 10 + 50 * sqrt(sqrt(Time::runtime));
	health = max_health;
	alive = true;
	align_verts();

	q.t0.verts[0].u = 0.0;
	q.t0.verts[0].v = 1.0;
	q.t0.verts[1].u = 1.0;
	q.t0.verts[1].v = 0.0;
	q.t0.verts[2].u = 0.0;
	q.t0.verts[2].v = 0.0;

	q.t1.verts[0].u = 1.0;
	q.t1.verts[0].v = 0.0;
	q.t1.verts[1].u = 1.0;
	q.t1.verts[1].v = 1.0;
	q.t1.verts[2].u = 0.0;
	q.t1.verts[2].v = 1.0;

	// printf("Added grid: %p, %p, %p\n", this, &game_grid, &grid);
}

void Enemy::update() {
	float x_dir, y_dir;
	bool at_goal;
	// int2 cell;

	if (position.x > 1) {
		position.x = 0.999;
	}
	if (position.x < -1) {
		position.x = -.999;
	}
	if (position.y > 1) {
		position.y = 0.999;
	}
	if (position.y < -1) {
		position.y = -.999;
	}

	game_grid->get_direction(position.x, position.y, x_dir, y_dir, at_goal);
	// printf("dir: %.2f, %.2f\n", x_dir, y_dir);
	if (at_goal) {
		// game_grid->remove_enemy(this);
		alive = false;
		Enemy::
		Enemy::remove_enemy(this);
	}
	float2 acc = float2(
		2.1*(x_dir - velocity.x/max_speed), 
		2.1*(y_dir - velocity.y/max_speed)
	);
	if (x_dir == 0 && y_dir == 0) {
		acc.x = .1f * ((float) rand() / RAND_MAX - .5f);
		acc.y = .1f * ((float) rand() / RAND_MAX - .5f);
	}
	velocity.x +=  acc.x * Time::delta;
	velocity.y +=  acc.y * Time::delta;
	position.x += velocity.x * Time::delta;
	position.y += velocity.y * Time::delta;

	game_grid->get_cell(position, cell);
	bool x_inside = cell.x >= 0 && cell.x < game_grid->field.size.x;
	bool y_inside = cell.y >= 0 && cell.y < game_grid->field.size.y;
	bool current = x_inside && y_inside;
	bool inside = current;
	bool last = last_cell.x >= 0 && last_cell.x < game_grid->field.size.x
		&& last_cell.y >= 0 && last_cell.y < game_grid->field.size.y;
	if (current) {
		current &= game_grid->field.filled[cell.y * game_grid->field.size.x + cell.x];
	}
	if (last) {
		last &= game_grid->field.filled[last_cell.y * game_grid->field.size.x + last_cell.x];
	}
	if ((current && !last)
		|| cell.x < 0 || cell.x > game_grid->field.size.x - 1
		|| cell.y < 0 || cell.y > game_grid->field.size.y - 1
		|| (game_grid->field.directions[cell.y * game_grid->field.size.x + cell.x] == ZERO_DIR
		&& cell != game_grid->field.end)) {

		position.x -= velocity.x * Time::delta;
		position.y -= velocity.y * Time::delta;
		bool x_diff = cell.x != last_cell.x;
		bool y_diff = cell.y != last_cell.y;
		if (x_diff & !y_diff) {
			velocity.x = -.5 * velocity.x;
		}
		if (y_diff & !x_diff) {
			velocity.y = -.5 * velocity.y;
		}
		if (x_diff & y_diff & inside) {
			bool first_neighbor = game_grid->field.filled[last_cell.y * game_grid->field.size.x + cell.x];
			bool second_neighbor = game_grid->field.filled[cell.y * game_grid->field.size.x + last_cell.x];
			if (first_neighbor & second_neighbor) {
				velocity.x = -.5 * velocity.x;
				velocity.y = -.5 * velocity.y;
			} else if (!first_neighbor & !second_neighbor) {
				velocity.x = -.5 * velocity.x;
				velocity.y = -.5 * velocity.y;
			} else if (first_neighbor & !second_neighbor) {
				velocity.x = -.5 * velocity.x;
			} else if (!first_neighbor & second_neighbor) {
				velocity.y = -.5 * velocity.y;
			}

		} else if (x_diff & y_diff & !inside) {
			if (x_inside & !y_inside) {
				velocity.y = -.5 * velocity.y;				
			} else if (!x_inside & y_inside) {
				velocity.x = -.5 * velocity.x;								
			} else if (!x_inside & !y_inside) {
				velocity.x = -.5 * velocity.x;								
				velocity.y = -.5 * velocity.y;
			}
		}
		position.x += velocity.x * Time::delta;
		position.y += velocity.y * Time::delta;

	}
	game_grid->get_cell(position, cell);
	if (cell != last_cell) {
		game_grid->change_cell(this);
	}
	last_cell = cell;
}

void Enemy::late_update() {
}

void Enemy::draw(GLFWwindow* window, GLuint program, GLint mvp_location) const {
	// std::cout << "drawing: " << name << std::endl;

	align_verts();
	enemy_quads[pool_idx] = q;

	// mat4x4 m;
	// mat4x4_identity(m);
	// m[3][0] = game_grid->field.world_center.x + position.x;
	// m[3][1] = game_grid->field.world_center.y + position.y;

	// Rendering instance = Rendering::get_instance();
	// glBufferData(GL_ARRAY_BUFFER, sizeof(q.t0.verts), q.t0.verts, GL_STATIC_DRAW);
	// Rendering::get_instance().draw_triangle(window, program, mvp_location, m);
	// instance.draw_triangle(window, program, mvp_location, m);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(q.t1.verts), q.t1.verts, GL_STATIC_DRAW);
	// Rendering::get_instance().draw_triangle(window, program, mvp_location, m);	
	// instance.draw_triangle(window, program, mvp_location, m);	
}

void Enemy::draw_enemies(GLFWwindow* window, GLuint program, GLint mvp_location) {
	// printf("Drawing %d enemies (%lu, %lu)\n", Enemy::active_enemies, sizeof(quad) * active_enemies, sizeof(quad));
	for (int i = 0; i < active_enemies; i++) {
		// std::cout << enemy_quads[i] << std::endl;
		// printf("pos: %.3f, %.3f\n", enemy_pool[i]->position.x, enemy_pool[i]->position.y);
		// printf("health: %.3f, %.3f\n", enemy_pool[i]->health, enemy_pool[i]->max_health);
		enemy_pool[i]->draw(window, program, mvp_location);
	}	


	float ratio;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float) height;
	glViewport(0, 0, width, height);

	// GLuint
	// glGenBuffers(1, )
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad) * active_enemies, enemy_quads.data(), GL_STATIC_DRAW);
	mat4x4 p, mvp, m;
	mat4x4_identity(m);
	m[3][0] = Enemy::game_grid->field.world_center.x;
	m[3][1] = Enemy::game_grid->field.world_center.y;
	// float ratio = 1.f;
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(mvp, p, m);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

	glDrawArrays(GL_TRIANGLES, 0, 1 * sizeof(quad) * active_enemies);
	// glDrawArrays(GL_TRIANGLES, 0, 3 * active_enemies);


}

void Enemy::add_enemy(float2 center, float2 size, GameGrid* grid) {

	if (active_enemies < pool_size) {
		Enemy *e = enemy_pool[active_enemies];
		e->configure(center, size, grid);
		active_enemies++;
		// return e;
	} else {
		printf("ERROR\n");
	}
/*
	if (enemy_pool.empty()) {
		return new Enemy(center, size, grid);
	} else {
		Enemy* e = enemy_pool.back();
		e->configure(center, size, grid);
		enemy_pool.pop_back();
		return e;
	}
*/
}

void Enemy::update_enemies() {
	// std::vector<int> counts(1000);
	for (int i = 0; i < active_enemies; i++) {
		enemy_pool[i]->update();
		if (enemy_pool[i]->pool_idx != i) {
			printf("ERROR: %d != %d\n", enemy_pool[i]->pool_idx, i);
		}
	}
}

void Enemy::late_update_enemies() {
	for (int i = 0; i < active_enemies; i++) {
		enemy_pool[i]->late_update();
	}
}

void Enemy::create_enemy_pool(int n, GameGrid* grid) {
	// Enemy(float2 center, float2 size, GameGrid* grid);
	for (int i = 0; i < n; i++) {

		Enemy *e = new Enemy(float2(0, 0), float2(0, 0), grid);
		e->name = "test: " + std::to_string(i);
		e->pool_idx = i;
		enemy_pool.push_back(e);
	}
	enemy_quads.resize(n);
	// for (int i = 0; i < n; i++) {
		// std::cout << "enemy: " << enemy_pool[i].name << std::endl;
		// printf("enemy: %s\n", enemy_pool[i].name);
	// }
	// enemy_pool(n, Enemy(float2(0, 0), float2(0, 0), NULL));
	pool_size = n;
	active_enemies = 0;
}

void Enemy::remove_enemy(Enemy* e) {
	// printf("active_enemies: %d\tenemy_idx: %d\n", active_enemies, e->pool_idx);
	Enemy* temp = enemy_pool[active_enemies-1];
	enemy_pool[active_enemies-1] = e;
	enemy_pool[e->pool_idx] = temp;
	temp->pool_idx = e->pool_idx;
	e->pool_idx = active_enemies - 1;
	active_enemies--;
	// enemy_pool.push_back(e);
}

void Enemy::apply_damage(float damage) {
	health -= damage;
	alive = health > 0;
	// printf("health: %.2f, %d\n", health, alive);
}