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
	if (main_script->frame % 1 == 0) {
		for (int i = 0; i < 10; i++) {
			Enemy::add_enemy(float2(-.98, .98), float2(0.01, 0.01), this);
		}
	}
	Tower::update_towers();
	Enemy::update_enemies();
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

