#pragma once
#include <cmath>
#include <vector>

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

template<class t>
struct vec4
{
	union
	{
		t raw[4];
		struct { t x, y, z, w; };
	};
	vec4() :x(0), y(0), z(0), w(0) {}
	vec4(vec3<t> vc3, t _w) :x(vc3.x), y(vc3.y), z(vc3.z), w(_w) {}
	vec3<t> xyz() { return vec3<t>(x, y, z); }
};

typedef vec4<int> vec4i;
typedef vec4<float> vec4f;

class Matrix
{
public:
	Matrix();
	Matrix(int dim);
	Matrix(int dim, float defaultValue);
	Matrix(float pitch, float yaw, float roll);
	Matrix(vec3f target);
	Matrix(vec3f camPos, vec3f camRot);
	~Matrix();

	void SetValue(int x, int y, float value);
	float GetValue(int x, int y);
	std::vector<std::vector<float>> GetRaw();

	static Matrix MakeProjectionMatrix(int f, int n, float fov, float aspect);

	vec4f MultipleVec4(vec4f v);
	vec3f MultipleVec3(vec3f v);

	Matrix MultipleMat(Matrix m);

	Matrix inverse();

	double d2r(float degrees);

private:
	std::vector<std::vector<float>> raw;
};