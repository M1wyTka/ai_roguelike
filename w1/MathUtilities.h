#pragma once
#include <cmath>

template<typename T>
T sqr(T a) { return a * a; }

template<typename T, typename U>
static float dist_sq(const T& lhs, const U& rhs) 
{
	return static_cast<float>(sqr(lhs.x - rhs.x) + sqr(lhs.y - rhs.y));
}

template<typename T, typename U>
static float dist(const T& lhs, const U& rhs) 
{ 
	return std::sqrtf(dist_sq(lhs, rhs));
}
