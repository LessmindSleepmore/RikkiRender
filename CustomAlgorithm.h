#pragma once
#include "GeometricTypes.h"

template <class t>
t max(t a, t b) {
	return a > b ? a : b;
};

template <class t>
t min(t a, t b) {
	return a > b ? b : a;
};

vec3f cross(vec3f a, vec3f b);

float dot(vec3f a, vec3f b);

vec3f normalize(vec3f a);
