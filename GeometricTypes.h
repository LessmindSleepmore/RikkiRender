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
	inline vec3<t> operator +(const vec3<t>& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	inline vec3<t> operator -(const vec3<t>& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
};

typedef vec3<int> vec3i;
typedef vec3<float> vec3f;

template <class t>
struct vec2
{
	union
	{
		t raw[2];
		struct { t x, y; };
	};
	vec2() :x(0), y(0){}
	vec2(t ix, t iy) :x(ix), y(iy){}
};

typedef vec2<int> vec2i;
typedef vec2<float> vec2f;

template<class t>
void swapVec2(vec2<t> &a, vec2<t> &b) {
	t temp = a.x;
	a.x = b.x;
	b.x = temp;
	temp = a.y;
	a.y = b.y;
	b.y = temp;
}