#include "CustomAlgorithm.h"
#include <math.h>

vec3f cross(vec3f a, vec3f b) {
	return vec3f(a.y * b.z - b.y * a.z, a.x * b.z - b.x * a.z, a.x * b.y - b.x * a.y);
};

float dot(vec3f a, vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
};

vec3f normalize(vec3f a) {
	float mlength = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	return vec3f(a.x / mlength, a.y / mlength, a.z / mlength);
};