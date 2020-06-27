#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unordered_set>
#include <queue>
#include <bits/stdc++.h> 

#include "linmath.h"
#include "game_classes.h"
#include "rendering.h"
#include "helper_structs.h"
#include "tower.h"
#include "enemy.h"


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
