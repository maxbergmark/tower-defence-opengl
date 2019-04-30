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

Tower::Tower(float2 center, float2 size, GameGrid* grid) {
	// printf("c.x: %.2f, c.y: %.2f, s.x: %.2f, s.y: %.2f\n", center.x, center.y, size.x, size.y);

	position = center - grid->field.world_center;
	q.t0.verts[0] = {-.5f * size.x,  .5f * size.y, 1.f, 0.f, 0.f};
	q.t0.verts[1] = { .5f * size.x, -.5f * size.y, 1.f, 0.f, 0.f};
	q.t0.verts[2] = {-.5f * size.x, -.5f * size.y, 0.f, 0.f, 1.f};

	q.t1.verts[0] = { .5f * size.x, -.5f * size.y, 1.f, 0.f, 0.f};
	q.t1.verts[1] = { .5f * size.x,  .5f * size.y, 0.f, 0.f, 1.f};
	q.t1.verts[2] = {-.5f * size.x,  .5f * size.y, 1.f, 0.f, 0.f};
	current_target = NULL;
	game_grid = grid;
	game_grid->get_cell(position, cell);
	range = 0.3;
	damage = 2.0;

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
				game_grid->remove_enemy(current_target);
				Enemy::return_enemy(current_target);
				current_target = NULL;
			}
		} else {
			current_target = NULL;
		}
	}
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
	for (int i = -3; i < 3; i++) {
		for (int j = -3; j < 3; j++) {
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
		vert line[2];

		line[0] = {
			position.x, 
			position.y,
			.2f, .9f, .2f
		};
		line[1] = {
			current_target->position.x, 
			current_target->position.y,
			.2f, .9f, .2f
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

	}
}


Enemy::Enemy(float2 center, float2 size, GameGrid* grid) {
	configure(center, size, grid);
}

void Enemy::configure(float2 center, float2 size, GameGrid* grid) {
	q.t0.verts[0] = {0*center.x - .5f * size.x, 0*center.y + .5f * size.y, 0.f, 1.f, 0.f};
	q.t0.verts[1] = {0*center.x + .5f * size.x, 0*center.y - .5f * size.y, 0.f, 1.f, 0.f};
	q.t0.verts[2] = {0*center.x - .5f * size.x, 0*center.y - .5f * size.y, 0.f, 1.f, 0.f};

	q.t1.verts[0] = {0*center.x + .5f * size.x, 0*center.y - .5f * size.y, 0.f, 1.f, 0.f};
	q.t1.verts[1] = {0*center.x + .5f * size.x, 0*center.y + .5f * size.y, 0.f, 1.f, 0.f};
	q.t1.verts[2] = {0*center.x - .5f * size.x, 0*center.y + .5f * size.y, 0.f, 1.f, 0.f};

	position = center;// - grid->field.world_center;
	velocity = float2(0, 0);

	game_grid = grid;
	max_speed = 0.5f;
	max_health = 10 * sqrt(Time::runtime);
	health = max_health;
	alive = true;
}

void Enemy::update() {
	float x_dir, y_dir;
	bool at_goal;
	// int2 cell;

	game_grid->get_direction(position.x, position.y, x_dir, y_dir, at_goal);
	if (at_goal) {
		game_grid->remove_enemy(this);
		alive = false;
		Enemy::return_enemy(this);
	}
	float2 acc = float2(
		1*(x_dir - velocity.x/max_speed), 
		1*(y_dir - velocity.y/max_speed)
	);
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
	for (int i = 0; i < 3; i++) {
		q.t0.verts[i].r = 1 - health / max_health;
		q.t0.verts[i].g = health / max_health;

		q.t1.verts[i].r = 1 - health / max_health;
		q.t1.verts[i].g = health / max_health;
	}
	mat4x4 m;
	mat4x4_identity(m);
	m[3][0] = game_grid->field.world_center.x + position.x;
	m[3][1] = game_grid->field.world_center.y + position.y;

	glBufferData(GL_ARRAY_BUFFER, sizeof(q.t0.verts), q.t0.verts, GL_STATIC_DRAW);
	Rendering::get_instance().draw_triangle(window, program, mvp_location, m);
	glBufferData(GL_ARRAY_BUFFER, sizeof(q.t1.verts), q.t1.verts, GL_STATIC_DRAW);
	Rendering::get_instance().draw_triangle(window, program, mvp_location, m);	
}

Enemy* Enemy::get_enemy(float2 center, float2 size, GameGrid* grid) {
	if (enemy_pool.empty()) {
		return new Enemy(center, size, grid);
	} else {
		Enemy* e = enemy_pool.back();
		e->configure(center, size, grid);
		enemy_pool.pop_back();
		return e;
	}
}

void Enemy::return_enemy(Enemy* e) {
	enemy_pool.push_back(e);
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
	field.start = int2(0, x-1);
	field.end = int2(x-1, 0);
	main_script = script;
	calculate_directions();
}

void GameGrid::update() {
	// printf("10\n");
	if (main_script->frame % 1 == 0) {
		Enemy* e = Enemy::get_enemy(float2(-.98, .98), float2(0.02, 0.02), this);
		e->name = "Enemy";
		main_script->game_objects.push_back(e);
	}
	// printf("20\n");
}

void GameGrid::late_update() {

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
			.2f, .2f, .2f
		};
		grid_h[2*i+1] = {
			field.right_x, 
			field.bottom_y + (field.top_y - field.bottom_y) * i / (float)(field.size.y), 
			.2f, .2f, .2f
		};
	}
	for (int i = 0; i < field.size.x+1; i++) {
		grid_v[2*i] = {
			field.left_x + (field.right_x - field.left_x) * i / (float)(field.size.x),
			field.bottom_y, 
			.2f, .2f, .2f
		};
		grid_v[2*i+1] = {
			field.left_x + (field.right_x - field.left_x) * i / (float)(field.size.x),
			field.top_y, 
			.2f, .2f, .2f
		};		
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_h), grid_h, GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2*field.size.y+2);

	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_v), grid_v, GL_STATIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2*field.size.x+2);
}


struct int2_pair {
	int2 node;
	int2_pair* prev;

    size_t operator()(void) const {}
    bool operator()(const int2_pair &other) const {}

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
			printf("node is null: %d, %d\n", current.node.x, current.node.y);
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
	printf("directions complete\n");
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

	int2_pair *end_node;
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
			end_node = node;
		}
	}
	// set_directions(end_node, field, visited);
	set_directions_new(field, s);
	// printf("nodes left: %lu\n", visited.size());
	// break;

}

void GameGrid::find_cell(float xpos, float ypos,
	float2 &center, float2 &size, bool &should_place) {

	xpos -= field.world_center.x;
	ypos -= field.world_center.y;

	int x_cell = (int) (field.size.x * .5f * (xpos + 1.f));
	int y_cell = (int) (field.size.y * .5f * (ypos + 1.f));
	bool is_filled = field.filled[field.size.x * y_cell + x_cell];
	if (x_cell < 0 || x_cell > field.size.x - 1 || y_cell < 0 || y_cell > field.size.y - 1 || is_filled) {
		should_place = false;
		return;
	}
	should_place = true;

	field.filled[field.size.x * y_cell + x_cell] = true;

	center.x = -1.f + (2.f * x_cell + 1.f) / (float)(field.size.x) + field.world_center.x;
	center.y = -1.f + (2.f * y_cell + 1.f) / (float)(field.size.y) + field.world_center.y;
	size.x = 2.f / (float)(field.size.x);
	size.y = 2.f / (float)(field.size.y);

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
	std::unordered_set<Enemy*>* new_cell = get_enemies(e->cell);
	if (contains(new_cell, e)) {
		new_cell->erase(new_cell->find(e));
	}

}

void GameGrid::get_direction(float xpos, float ypos,
	float &x_dir, float &y_dir, bool &at_goal) {

	// xpos -= field.world_center.x;
	// ypos -= field.world_center.y;

	int x_cell = (int) (field.size.x * .5f * (xpos + 1.f));
	int y_cell = (int) (field.size.y * .5f * (ypos + 1.f));
	if (x_cell < 0 || x_cell > field.size.x-1 || y_cell < 0 || y_cell > field.size.x - 1) {
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

void GameGrid::create_tower(GLFWwindow* window, float xpos, float ypos, std::vector<GameObject*> &game_objects) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	xpos = (xpos - width/2) / (float) (height/2);
	ypos = (ypos - height/2) / (float) (height/2);

	float2 center, size;
	bool should_place;
	ypos *= -1;
	find_cell(xpos, ypos, center, size, should_place);

	if (should_place) {
		Tower* t = new Tower(center, size, this);
		t->name = "Tower";
		game_objects.push_back(t);
		calculate_directions();
	}


}

std::unordered_set<Enemy*>* GameGrid::get_enemies(int2 cell) {
	return enemies[cell.y * field.size.x + cell.x];
}


void GameGrid::change_cell(Enemy* e) {
	std::unordered_set<Enemy*>* old_cell = get_enemies(e->last_cell);
	std::unordered_set<Enemy*>* new_cell = get_enemies(e->cell);
	if (contains(old_cell, e)) {
		old_cell->erase(old_cell->find(e));
	}
	new_cell->insert(e);
	// printf("moving enemy: %lu/%lu\n", old_cell->size(), new_cell->size());

}
