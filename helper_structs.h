#pragma once

#include <unordered_set>
#include <iostream>
#include <iomanip>
// #include <unordered_set>
#include <queue>

#define LEFT_DIR 33
#define RIGHT_DIR 35
#define UP_DIR 50
#define DOWN_DIR 18
#define ZERO_DIR 34


struct float2 {
	float x;
	float y;

	float2();
	float2(float xt, float yt);
	float magnitude();
	float2 operator + (const float2 b);
	float2 operator - (const float2 b);
	float2& operator += (const float2& b);
	float2& operator -= (const float2& b);
};

struct int2 {
	int x;
	int y;

	int2();
	int2(int xt, int yt);
	int2 operator + (const int2 b);
	int2& operator += (const int2& b);
	int2& operator -= (const int2& b);
	bool operator==(const int2& other) const;
	bool operator!=(const int2& other) const;
	int2 operator-(const int2 &b) const;
};


namespace std {
	template<>
	struct hash<int2> {
		inline size_t operator()(const int2& x) const {
			size_t value = (x.y * 13) ^ (x.x * 23) 
				^ (x.x * 53) ^ (x.y * 67);
			return value;
		}
	};
}

struct vert {
	mutable float x, y;
	mutable float r, g, b;
	mutable float u, v;
};

inline std::ostream& operator<<(std::ostream& os, const vert& v) {
    return os << std::setprecision(3) << std::fixed 
    << "x: " << v.x << ", y: " << v.y
    << "\tr: " << v.r << ", g: " << v.g << ", b: " << v.b;
    ; 
}

struct triangle {
	vert verts[3];
};

inline std::ostream& operator<<(std::ostream& os, const triangle& t) {
    return os << "\t\tv0: " <<  t.verts[0] 
    	<< "\n\t\tv1: " << t.verts[1]
    	<< "\n\t\tv2: " << t.verts[2];
}

struct quad {
	triangle t0;
	triangle t1;
};

inline std::ostream& operator<<(std::ostream& os, const quad& q) {
    return os << "{\n\tt0: \n" <<  q.t0 << "\n\tt1: \n" << q.t1 << "\n}"; 
}

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

