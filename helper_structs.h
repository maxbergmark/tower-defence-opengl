#pragma once
#include <unordered_set>
#include <iostream>
#include <iomanip>

struct float2 {
	float x;
	float y;
	float2() {
		x = 0;
		y = 0;
	}
	float2(float xt, float yt) {
		x = xt;
		y = yt;
	}

	float magnitude() {
		return sqrt(x*x + y*y);
	}

	float2 operator + (const float2 b) {
		float2 temp;
		temp.x = x + b.x;
		temp.y = y + b.y;
		return temp;
	}

	float2 operator - (const float2 b) {
		float2 temp;
		temp.x = x - b.x;
		temp.y = y - b.y;
		return temp;
	}

	float2& operator += (const float2& b) {
		x += b.x;
		y += b.y;
		return *this;
	}

	float2& operator -= (const float2& b) {
		x -= b.x;
		y -= b.y;
		return *this;
	}
};

struct int2 {
	int x;
	int y;

	// size_t operator()(void) const {}
	// bool operator()(const int2 &other) const {}

	int2() {
		x = 0;
		y = 0;
	}
	int2(int xt, int yt) {
		x = xt;
		y = yt;
	}
	
	inline int2 operator + (const int2 b) {
		int2 temp;
		temp.x = x + b.x;
		temp.y = y + b.y;
		return temp;
	}


	int2& operator += (const int2& b) {
		x += b.x;
		y += b.y;
		return *this;
	}

	int2& operator -= (const int2& b) {
		x -= b.x;
		y -= b.y;
		return *this;
	}

	inline bool operator==(const int2& other) const {
		bool comparison = (x == other.x) & (y == other.y);
		return comparison;
	}

	inline bool operator!=(const int2& other) const {
		bool comparison = (x != other.x) | (y != other.y);
		return comparison;
	}
};

inline int2 operator - (const int2 &a, const int2 &b) {
	int2 temp;
	temp.x = a.x - b.x;
	temp.y = a.y - b.y;
	return temp;
}

namespace std {
	template<>
	struct hash<int2> {
		inline size_t operator()(const int2& x) const {
			size_t value = (x.y * 13) ^ (x.x * 23) ^ (x.x * 53) ^ (x.y * 67);
			return value;
		}
	};
}

struct vert {
	mutable float x, y;
	mutable float r, g, b;
	mutable float u, v;
/*
	std::string toString() {
		return "x: " + std::to_string(x)
			+ "y: " + std::to_string(y);
	}
*/
};

inline std::ostream& operator<<(std::ostream& os, const vert& v) {
    return os << std::setprecision(3) << std::fixed 
    << "x: " << v.x << ", y: " << v.y
    << "\tr: " << v.r << ", g: " << v.g << ", b: " << v.b;
    ; 
}

struct triangle {
	vert verts[3];
/*
	std::string toString() {
		return "v0: " + 
			verts[0].toString() + 
			"\n v1: " + 
			verts[1].toString() +
			"\n v2: " + 
			verts[2].toString();
	}
*/
};

inline std::ostream& operator<<(std::ostream& os, const triangle& t) {
    return os << "\t\tv0: " <<  t.verts[0] 
    	<< "\n\t\tv1: " << t.verts[1]
    	<< "\n\t\tv2: " << t.verts[2];
}

struct quad {
	triangle t0;
	triangle t1;
/*	
	std::string toString() {
		return "t0: " + 
			t0.toString() + 
			"\n t1: " + 
			t1.toString();
	}
*/
};

inline std::ostream& operator<<(std::ostream& os, const quad& q) {
    return os << "{\n\tt0: \n" <<  q.t0 << "\n\tt1: \n" << q.t1 << "\n}"; 
}


struct tower {
	quad q;
	float speed;
	float damage;
};

struct cell {
	tower t;
};

struct playfield {
	int2 start;
	int2 end;
	float left_x = -1.f;
	float right_x = 1.f;
	float top_y = 1.f;
	float bottom_y = -1.f;
	mutable float2 world_center;
	int2 size;
	bool *filled;
	char *directions;
};
