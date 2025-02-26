#pragma once

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
	inline float operator *(const vec2<t>& v) const { return x * v.x + y * v.y; }
	inline vec2<t> operator -(const vec2<t>& v) const { return vec2<t>(x - v.x, y - v.y); }
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

template <class t>
struct vec3
{
	union
	{
		t raw[3];
		struct { t x, y, z; };
	};
	vec3() :x(0), y(0), z(0) {}
	vec3(t ix, t iy, t iz) :x(ix), y(iy), z(iz) {}
	inline vec3<t> operator +(const vec3<t>& v) const { return vec3(x + v.x, y + v.y, z + v.z); }
	inline vec3<t> operator -(const vec3<t>& v) const { return vec3(x - v.x, y - v.y, z - v.z); }
	inline vec3<t> operator ^(const vec3<t>& v) const { return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	inline float operator *(const vec3<t>& v) const { return x * v.x + y * v.y + z * v.z; }
	inline vec3<t> operator /(const float f) const { return vec3(x / f, y / f, z / f); }
	inline vec2<t> xy() const { return vec2<t>(x, y); }
	inline vec3<t> normalize() const {
		float _f = sqrt(x * x + y * y + z * z);
		return vec3<t>(x / _f, y / _f, z / _f);
	}
};

typedef vec3<int> vec3i;
typedef vec3<float> vec3f;