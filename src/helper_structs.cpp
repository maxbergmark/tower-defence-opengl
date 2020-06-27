#include <iostream>
#include <iomanip>

#include "helper_structs.h"
#include "linmath.h"

float2::float2() {
	x = 0;
	y = 0;
}
float2::float2(float xt, float yt) {
	x = xt;
	y = yt;
}

float float2::magnitude() {
	return sqrt(x*x + y*y);
}

float2 float2::operator + (const float2 b) {
	float2 temp;
	temp.x = x + b.x;
	temp.y = y + b.y;
	return temp;
}

float2 float2::operator - (const float2 b) {
	float2 temp;
	temp.x = x - b.x;
	temp.y = y - b.y;
	return temp;
}

float2& float2::operator += (const float2& b) {
	x += b.x;
	y += b.y;
	return *this;
}

float2& float2::operator -= (const float2& b) {
	x -= b.x;
	y -= b.y;
	return *this;
}


int2::int2() {
	x = 0;
	y = 0;
}

int2::int2(int xt, int yt) {
	x = xt;
	y = yt;
}

int2 int2::operator + (const int2 b) {
	int2 temp;
	temp.x = x + b.x;
	temp.y = y + b.y;
	return temp;
}

int2& int2::operator += (const int2& b) {
	x += b.x;
	y += b.y;
	return *this;
}

int2& int2::operator -= (const int2& b) {
	x -= b.x;
	y -= b.y;
	return *this;
}

bool int2::operator==(const int2& other) const {
	bool comparison = (x == other.x) & (y == other.y);
	return comparison;
}

bool int2::operator!=(const int2& other) const {
	bool comparison = (x != other.x) | (y != other.y);
	return comparison;
}


int2 int2::operator-(const int2 &b) const {
	int2 temp;
	temp.x = x - b.x;
	temp.y = y - b.y;
	return temp;
}

/*
namespace std {
	template<>
	struct hash<int2> {
		inline size_t operator()(const int2& x) const {
			size_t value = (x.y * 13) ^ (x.x * 23) ^ (x.x * 53) ^ (x.y * 67);
			return value;
		}
	};
}
*/