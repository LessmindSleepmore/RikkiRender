#pragma once


template <class t> 
struct vec3
{
	union
	{
		t raw[3];
		struct { t x, y, z; };
	};
	vec3():x(0), y(0), z(0) {}
	vec3(t ix, t iy, t iz):x(ix), y(iy), z(iz) {}
};

typedef vec3<int> vec3i;
typedef vec3<float> vec3f;