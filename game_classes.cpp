#include "linmath.h"
#include "game_classes.h"
#include "rendering.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "helper_structs.h"
#include <unordered_set>
#include <queue>
#include<bits/stdc++.h> 

#define LEFT_DIR 33
#define RIGHT_DIR 35
#define UP_DIR 50
#define DOWN_DIR 18
#define ZERO_DIR 34


std::vector<Enemy*> Enemy::enemy_pool;
std::vector<quad> Enemy::enemy_quads;
int Enemy::pool_size = 0;
int Enemy::active_enemies = 0;
GameGrid* Enemy::game_grid;

std::vector<Tower*> Tower::tower_pool;
int Tower::pool_size = 0;
int Tower::active_towers = 0;


Tower::Tower(float2 center, float2 size, GameGrid* grid) {
	configure(center, size, grid);
}

void Tower::configure(float2 center, float2 size, GameGrid* grid) {
	position = center - grid->field.world_center;
	q.t0.verts[0] = {-.5f * size.x,  .5f * size.y, 1.f, 0.f, 0.f};
	q.t0.verts[1] = { .5f * size.x, -.5f * size.y, 1.f, 0.f, 0.f};
	q.t0.verts[2] = {-.5f * size.x, -.5f * size.y, 0.f, 0.f, 1.f};

	q.t1.verts[0] = { .5f * size.x, -.5f * size.y, 1.f, 0.f, 0.f};
	q.t1.verts[1] = { .5f * size.x,  .5f * size.y, 0.f, 0.f, 1.f};
	q.t1.verts[2] = {-.5f * size.x,  .5f * size.y, 1.f, 0.f, 0.f};

	q.t0.verts[0].u = 0.0 * .9f + .05f;
	q.t0.verts[0].v = 1.0 * .9f + .05f;
	q.t0.verts[1].u = 1.0 * .9f + .05f;
	q.t0.verts[1].v = 0.0 * .9f + .05f;
	q.t0.verts[2].u = 0.0 * .9f + .05f;
	q.t0.verts[2].v = 0.0 * .9f + .05f;

	q.t1.verts[0].u = 1.0 * .9f + .05f;
	q.t1.verts[0].v = 0.0 * .9f + .05f;
	q.t1.verts[1].u = 1.0 * .9f + .05f;
	q.t1.verts[1].v = 1.0 * .9f + .05f;
	q.t1.verts[2].u = 0.0 * .9f + .05f;
	q.t1.verts[2].v = 1.0 * .9f + .05f;

	current_target = NULL;
	game_grid = grid;
	game_grid->get_cell(position, cell);
	range = 0.3;
	damage = 20.0;
}

void Tower::update() {
	if (out_of_range()) {
		find_new_target();
	}
	if (frame++ % 1 == 0 && current_target != NULL) {
		if (current_target->alive) {
			current_target->apply_damage(damage);
			if (!current_target->alive) {
				current_target->alive = false;
				// game_grid->remove_enemy(current_target);
				// Enemy::return_enemy(current_target);
				Enemy::remove_enemy(current_target);
				current_target = NULL;
			}
		} else {
			current_target = NULL;
		}
	}
}


void Tower::update_towers() {
	for (int i = 0; i < active_towers; i++) {
		tower_pool[i]->update();
	}
}

void Tower::late_update_towers() {
	for (int i = 0; i < active_towers; i++) {
		tower_pool[i]->late_update();
	}
}

void Tower::draw_towers(GLFWwindow* window, GLuint program, GLint mvp_location) {
	// printf("Drawing %d towers\n", Tower::active_towers);
	for (int i = 0; i < active_towers; i++) {
		tower_pool[i]->draw(window, program, mvp_location);
	}
}

void Tower::create_tower_pool(int n, GameGrid* grid) {

	for (int i = 0; i < n; i++) {
		Tower *t = new Tower(float2(0, 0), float2(0, 0), grid);
		t->name = "tower: " + std::to_string(i);
		t->pool_idx = i;
		tower_pool.push_back(t);
	}
	pool_size = n;
	active_towers = 0;
}

Tower* Tower::add_tower(float2 center, float2 size, GameGrid* grid) {

	if (active_towers < pool_size) {
		Tower *t = tower_pool[active_towers];
		t->configure(center, size, grid);
		active_towers++;
		return t;
	} else {
		printf("ERROR\n");
		return NULL;
	}
}

void Tower::remove_tower(Tower* t) {
	Tower* temp = tower_pool[active_towers-1];
	tower_pool[active_towers-1] = t;
	temp->pool_idx = t->pool_idx;
	tower_pool[t->pool_idx] = temp;
	t->pool_idx = active_towers - 1;
	active_towers--;
}

bool Tower::out_of_range() {
	if (current_target == NULL) {
		return true;
	}
	return ((position - current_target->position).magnitude() > range);
}

void Tower::find_new_target() {
	// printf("finding new target\n");
	float min_dist = range;
	Enemy* new_target = NULL;
	int cell_dist = 3;
	for (int i = -cell_dist; i <= cell_dist; i++) {
		for (int j = -cell_dist; j <= cell_dist; j++) {
			int2 test = cell + int2(i, j);
			if (test.x >= 0 && test.y >= 0 
				&& test.x < game_grid->field.size.x
				&& test.y < game_grid->field.size.y) {

				std::unordered_set<Enemy*>* cell_enemies = game_grid->get_enemies(test);
				// printf("size: %lu\n", cell_enemies->size());
				for (const auto& e : *cell_enemies) {
					float dist = (e->position - position).magnitude();
					// printf("dist: %.2f, %p\n", dist, e);
					if (dist < min_dist) {
						min_dist = dist;
						new_target = e;
					}
				}
			}
		}
	}
	current_target = new_target;
}

void Tower::late_update() {
}

void Tower::draw(GLFWwindow* window, GLuint program, GLint mvp_location) const {
	// printf("drawing tower\n");
	mat4x4 m;
	mat4x4_identity(m);
	m[3][0] = game_grid->field.world_center.x + position.x;
	m[3][1] = game_grid->field.world_center.y + position.y;

	glBufferData(GL_ARRAY_BUFFER, sizeof(q.t0.verts), q.t0.verts, GL_STATIC_DRAW);
	Rendering::get_instance().draw_triangle(window, program, mvp_location, m);
	glBufferData(GL_ARRAY_BUFFER, sizeof(q.t1.verts), q.t1.verts, GL_STATIC_DRAW);
	Rendering::get_instance().draw_triangle(window, program, mvp_location, m);

	if (current_target != NULL) {
/*
		q.t0.verts[0].g = 1.f;
		q.t0.verts[1].g = 1.f;
		q.t1.verts[0].g = 1.f;
		q.t1.verts[2].g = 1.f;
*/		

		vert line[2];

		line[0] = {
			position.x, 
			position.y,
			.2f, .9f, .2f, .0f, .0f
		};
		line[1] = {
			current_target->position.x, 
			current_target->position.y,
			.2f, .9f, .2f, 1.f, 1.f
		};
		mat4x4 m;
		mat4x4_identity(m);
		m[3][0] = game_grid->field.world_center.x;
		m[3][1] = game_grid->field.world_center.y;
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;

		mat4x4 p, mvp;
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);

		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
		glDrawArrays(GL_LINES, 0, 2);

	} else {
/*
		q.t0.verts[0].g = 0.f;
		q.t0.verts[1].g = 0.f;
		q.t1.verts[0].g = 0.f;
		q.t1.verts[2].g = 0.f;
*/
	}
}


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
		5.1*(x_dir - velocity.x/max_speed), 
		5.1*(y_dir - velocity.y/max_speed)
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

GameGrid::GameGrid(int x, int y, Main* script) {

	field.size = int2(x, y);
	field.filled = new bool[x*y];
	field.directions = new char[x*y];
	enemies.resize(x*y);
	for (int i = 0; i < x*y; i++) {
		field.filled[i] = false;
		field.directions[i] = ZERO_DIR;
		enemies[i] = new std::unordered_set<Enemy*>;
	}
	field.start = int2(0, y-1);
	field.end = int2(x-1, 0);
	main_script = script;
	Enemy::game_grid = this;
	Tower::create_tower_pool(1000, this);
	Enemy::create_enemy_pool(100000, this);
	calculate_directions();
}

void GameGrid::update() {
	// printf("10\n");
	if (main_script->frame % 1 == 0) {
		// printf("enemies: %d\n", Enemy::active_enemies);
		// printf("Adding enemy: %d, %d\n", Enemy::active_enemies, Enemy::pool_size);
		for (int i = 0; i < 10; i++) {
			Enemy::add_enemy(float2(-.98, .98), float2(0.01, 0.01), this);
		}
/*
		Enemy* e = Enemy::get_enemy(float2(-.98, .98), float2(0.01, 0.01), this);
		e->name = "Enemy";
		main_script->game_objects.push_back(e);
*/
	}
	Tower::update_towers();
	Enemy::update_enemies();

	// printf("20\n");
}

void GameGrid::late_update() {
	Tower::late_update_towers();
	Enemy::late_update_enemies();

}

void GameGrid::draw(GLFWwindow* window, GLuint program, GLint mvp_location) const {

	float ratio;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float) height;
	field.world_center.x = ratio - 1;
	glViewport(0, 0, width, height);


	mat4x4 m, p, mvp;
	mat4x4_identity(m);

	m[3][0] = field.world_center.x;
	m[3][1] = field.world_center.y;

	mat4x4_ortho(p, 
		-ratio, 
		ratio, 
		-1.f, 
		1.f, 
		1.f, -1.f
	);

	mat4x4_mul(mvp, p, m);
	glUseProgram(program);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);


	vert grid_h[2*field.size.y+2];
	vert grid_v[2*field.size.x+2];

	for (int i = 0; i < field.size.y+1; i++) {
		grid_h[2*i] = {
			field.left_x, 
			field.bottom_y + (field.top_y - field.bottom_y) * i / (float)(field.size.y),
			.2f, .2f, .2f, .5f, .5f
		};
		grid_h[2*i+1] = {
			field.right_x, 
			field.bottom_y + (field.top_y - field.bottom_y) * i / (float)(field.size.y), 
			.2f, .2f, .2f, .5f, .5f
		};
	}
	for (int i = 0; i < field.size.x+1; i++) {
		grid_v[2*i] = {
			field.left_x + (field.right_x - field.left_x) * i / (float)(field.size.x),
			field.bottom_y, 
			.2f, .2f, .2f, .5f, .5f
		};
		grid_v[2*i+1] = {
			field.left_x + (field.right_x - field.left_x) * i / (float)(field.size.x),
			field.top_y, 
			.2f, .2f, .2f, .5f, .5f
		};		
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_h), grid_h, GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2*field.size.y+2);

	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_v), grid_v, GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2*field.size.x+2);

	Tower::draw_towers(window, program, mvp_location);
	Enemy::draw_enemies(window, program, mvp_location);
}


struct int2_pair {
	int2 node;
	int2_pair* prev;

    // size_t operator()(void) const {}
    // bool operator()(const int2_pair &other) const {}

	int2_pair() {
		prev = NULL;
	}
	int2_pair(int2 n, int2_pair* p) {
		node = n;
		prev = p;
	}

	int2 get_node() const {
		return node;
	}

	inline bool operator==(const int2_pair& other) const {
        bool comparison = (node == other.node);
        return comparison;
    }
};

namespace std {
    template<>
    struct hash<int2_pair> {
        inline size_t operator()(const int2_pair& x) const {
        	// return hash<int2>(x.node);
            size_t value = (x.node.y * 13) ^ (x.node.x * 23) ^ (x.node.x * 53) ^ (x.node.y * 67);
            return value;
        }
    };
}

inline bool contains(std::unordered_set<int2_pair> &s, std::unordered_set<int2_pair>::const_iterator &got, int2 &node) {
	got = s.find(int2_pair(node, NULL));
	return got != s.end();
}

inline bool contains(std::unordered_set<int2> &s, std::unordered_set<int2>::const_iterator &got, int2 &node) {
	got = s.find(node);
	return got != s.end();
}

bool in_bounds(playfield field, int2 node) {
	return (node.x >= 0) & (node.x < field.size.x) & (node.y >= 0) & (node.y < field.size.y);
}

void add_neighbors(int2_pair *node,
	std::queue<int2_pair*> &q,
	std::unordered_set<int2_pair> &s,
	std::unordered_set<int2_pair>::const_iterator &got,
	playfield field
) {
	int2_pair *n_west = new int2_pair;
	n_west->node = node->node + int2(-1, 0);
	n_west->prev = node;
	if (in_bounds(field, n_west->node) && !contains(s, got, n_west->node) 
		&& !field.filled[n_west->node.y * field.size.x + n_west->node.x]) {
		q.push(n_west);
	}

	int2_pair *n_east = new int2_pair;
	n_east->node = node->node + int2(1, 0);
	n_east->prev = node;
	if (in_bounds(field, n_east->node) && !contains(s, got, n_east->node) 
		&& !field.filled[n_east->node.y * field.size.x + n_east->node.x]) {
		q.push(n_east);
	}

	int2_pair *n_south = new int2_pair;
	n_south->node = node->node + int2(0, -1);
	n_south->prev = node;
	if (in_bounds(field, n_south->node) && !contains(s, got, n_south->node) 
		&& !field.filled[n_south->node.y * field.size.x + n_south->node.x]) {
		q.push(n_south);
	}

	int2_pair *n_north = new int2_pair;
	n_north->node = node->node + int2(0, 1);
	n_north->prev = node;
	if (in_bounds(field, n_north->node) && !contains(s, got, n_north->node) 
		&& !field.filled[n_north->node.y * field.size.x + n_north->node.x]) {
		q.push(n_north);
	}	
}

void set_directions(int2_pair *end_node, 
	playfield field, 
	std::unordered_set<int2> &visited) {

	int2_pair *current = end_node;

	while (current->prev != NULL) {
		int2_pair *next = current->prev;
		int2 dir = current->node - next->node;
		if (dir.x == 0 && dir.y == -1) {
			field.directions[next->node.y * field.size.x + next->node.x] = DOWN_DIR;
		} else if (dir.x == 0 && dir.y == 1) {
			field.directions[next->node.y * field.size.x + next->node.x] = UP_DIR;
		} else if (dir.x == -1 && dir.y == 0) {
			field.directions[next->node.y * field.size.x + next->node.x] = LEFT_DIR;
		} else if (dir.x == 1 && dir.y == 0) {
			field.directions[next->node.y * field.size.x + next->node.x] = RIGHT_DIR;
		} else {
			printf("error: %d, %d (%d, %d)(%d, %d), %p, %p, %p\n", 
				dir.x, dir.y, current->node.x, current->node.y, next->node.x, 
				next->node.y, current, current->prev, next);
		}

		current = current->prev;
	}
}

void set_directions_new(playfield &field, std::unordered_set<int2_pair> &pairs) {
	for (const auto& current : pairs) {
		if (current.prev == NULL) {
			// printf("node is null: %d, %d\n", current.node.x, current.node.y);
			continue;
		}
		int2_pair next = *current.prev;
		int2 dir = next.node - current.node;
		if (dir.x == 0 && dir.y == -1) {
			// printf("dir: (%d, %d) (%d, %d) DOWN\n", current.node.x, current.node.y, next.node.x, next.node.y);
			field.directions[current.node.y * field.size.x + current.node.x] = DOWN_DIR;
		} else if (dir.x == 0 && dir.y == 1) {
			// printf("dir: (%d, %d) (%d, %d) UP\n", current.node.x, current.node.y, next.node.x, next.node.y);
			field.directions[current.node.y * field.size.x + current.node.x] = UP_DIR;
		} else if (dir.x == -1 && dir.y == 0) {
			// printf("dir: (%d, %d) (%d, %d) LEFT\n", current.node.x, current.node.y, next.node.x, next.node.y);
			field.directions[current.node.y * field.size.x + current.node.x] = LEFT_DIR;
		} else if (dir.x == 1 && dir.y == 0) {
			// printf("dir: (%d, %d) (%d, %d) RIGHT\n", current.node.x, current.node.y, next.node.x, next.node.y);
			field.directions[current.node.y * field.size.x + current.node.x] = RIGHT_DIR;
		} else {
			printf("error: %d, %d (%d, %d)(%d, %d), %p, %p, %p\n", 
				dir.x, dir.y, current.node.x, current.node.y, next.node.x, 
				next.node.y, &current, current.prev, &next);
		}

	}
	// printf("directions complete\n");
}

void GameGrid::calculate_directions() {
	int x = field.size.x;
	int y = field.size.y;
	for (int i = 0; i < x*y; i++) {
		field.directions[i] = ZERO_DIR;
		// visited.insert(*new int2(i / x, i % x));
	}

	std::queue<int2_pair*> q;
	std::unordered_set<int2_pair> s;
	std::unordered_set<int2_pair>::const_iterator got;

	int2_pair end_node;
	// int2 start = *begin(visited);
	// visited.erase(
		// visited.find(start)
	// );
	// int2_pair *start_node = new int2_pair(start, NULL);
	// q.push(start_node);
	q.push(new int2_pair(int2(field.end.x, field.end.y), NULL));
	while (!q.empty()) {
		int2_pair *node;
		do {
			node = q.front();
			q.pop();
		} while(contains(s, got, node->node) && !q.empty());

		s.insert(*node);
		add_neighbors(node, q, s, got, field);

		if (node->node.x == field.end.x && node->node.y == field.end.y) {
			end_node = *node;
		}
	}
	// set_directions(end_node, field, visited);
	set_directions_new(field, s);
	// printf("nodes left: %lu\n", visited.size());
	// break;

}

int2 GameGrid::find_cell(float xpos, float ypos,
	float2 &center, float2 &size, bool &should_place) {

	xpos -= field.world_center.x;
	ypos -= field.world_center.y;

	int x_cell = (int) (field.size.x * .5f * (xpos + 1.f));
	int y_cell = (int) (field.size.y * .5f * (ypos + 1.f));
	bool is_filled = field.filled[field.size.x * y_cell + x_cell];
	if (x_cell < 0 || x_cell > field.size.x - 1 || y_cell < 0 || y_cell > field.size.y - 1 || is_filled) {
		should_place = false;
		field.filled[field.size.x * y_cell + x_cell] = false;
		return int2(x_cell, y_cell);
	}
	should_place = true;

	field.filled[field.size.x * y_cell + x_cell] = true;

	center.x = -1.f + (2.f * x_cell + 1.f) / (float)(field.size.x) + field.world_center.x;
	center.y = -1.f + (2.f * y_cell + 1.f) / (float)(field.size.y) + field.world_center.y;
	size.x = 2.f / (float)(field.size.x);
	size.y = 2.f / (float)(field.size.y);

	return int2(x_cell, y_cell);

}

inline bool contains(std::unordered_set<Enemy*>* &s, Enemy* &node) {
	std::unordered_set<Enemy*>::const_iterator got;
	got = s->find(node);
	return got != s->end();
}

void GameGrid::remove_enemy(Enemy* e) {
	main_script->game_objects.erase(
		std::find(
			main_script->game_objects.begin(),
			main_script->game_objects.end(),
			e
		)
	);
	if (0 <= e->cell.y && e->cell.y < field.size.y && 0 <= e->cell.x && e->cell.x < field.size.x) {
		std::unordered_set<Enemy*>* new_cell = get_enemies(e->cell);
		if (contains(new_cell, e)) {
			new_cell->erase(new_cell->find(e));
		}
	}
}

void GameGrid::get_direction(float xpos, float ypos,
	float &x_dir, float &y_dir, bool &at_goal) {

	// xpos -= field.world_center.x;
	// ypos -= field.world_center.y;
	int x_cell = floor(field.size.x * .5f * (xpos + 1.f));
	int y_cell = floor(field.size.y * .5f * (ypos + 1.f));
	if (x_cell < 0 || x_cell > field.size.x-1 || y_cell < 0 || y_cell > field.size.y - 1) {
		printf("outside of grid: %d, %d\n", x_cell, y_cell);
		return;
	}
	at_goal = (x_cell == field.end.x && y_cell == field.end.y);
	char dir = field.directions[field.size.x * y_cell + x_cell];
	// printf("x: %d, y: %d, (%.3f, %.3f) (%.3f, %.3f)\n", x_cell, y_cell, xpos, ypos, x_dir, y_dir);
	x_dir = (dir & 0b11) - 2;
	y_dir = ((dir >> 4) & 0b11) - 2;

}

void GameGrid::get_cell(float2 pos, int2 &cell) {
	int x_cell = floor(field.size.x * .5f * (pos.x + 1.f));
	int y_cell = floor(field.size.y * .5f * (pos.y + 1.f));
	cell = int2(x_cell, y_cell);	
}

Tower* GameGrid::find_tower(int2 cell) {
	for (Tower* t : towers) {
		if (t->cell == cell) {
			// printf("found tower\n");
			return t;
		}
	}
	return NULL;
}

void GameGrid::create_tower(GLFWwindow* window, float xpos, float ypos, 
	std::vector<GameObject*> &game_objects) {
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	xpos = (xpos - width/2) / (float) (height/2);
	ypos = (ypos - height/2) / (float) (height/2);

	float2 center, size;
	bool should_place;
	ypos *= -1;
	int2 cell = find_cell(xpos, ypos, center, size, should_place);

	if (should_place) {
		Tower* t = Tower::add_tower(center, size, this);
		// Tower* t = new Tower(center, size, this);
		// t->name = "Tower";
		towers.push_back(t);
		// game_objects.push_back(t);
		calculate_directions();
	} else {
		// printf("Should remove\n");
		Tower* t = find_tower(cell);
		if (t != NULL) {
			// printf("tower: %p\n", t);
			Tower::remove_tower(t);
			towers.erase(std::find(
				towers.begin(),
				towers.end(),
				t
			));
			calculate_directions();
		}
	}
}

std::unordered_set<Enemy*>* GameGrid::get_enemies(int2 cell) {
	if (0 <= cell.y && cell.y < field.size.y && 0 <= cell.x && cell.x < field.size.x) {
		return enemies[cell.y * field.size.x + cell.x];
	}
	return NULL;
}


void GameGrid::change_cell(Enemy* e) {
	if (0 <= e->last_cell.y && e->last_cell.y < field.size.y && 0 <= e->last_cell.x && e->last_cell.x < field.size.x) {
		std::unordered_set<Enemy*>* old_cell = get_enemies(e->last_cell);
		if (contains(old_cell, e)) {
			old_cell->erase(old_cell->find(e));
		}
	}
	if (0 <= e->cell.y && e->cell.y < field.size.y && 0 <= e->cell.x && e->cell.x < field.size.x) {
		std::unordered_set<Enemy*>* new_cell = get_enemies(e->cell);
		new_cell->insert(e);
	}

}
