#pragma once

#include "GeometricTypes.h"
#include <math.h>
// 模板函数实例化时需要完整的定义

template <class t>
t max(t a, t b) {
	return a > b ? a : b;
};

template <class t>
t min(t a, t b) {
	return a > b ? b : a;
};

template <class t>
t cross(t a, t b) {
	return a ^ b;
}

template <class  t>
float dot(t a, t b) {
	return a * b;
};

template <class t>
t normalize(t a) {
	return a.normalize();
};


void RGB2HLS(unsigned char r, unsigned char g, unsigned char b, float& h, float& l, float& s);

void HLS2RGB(float h, float l, float s, unsigned char& r, unsigned char& g, unsigned char& b);

void decreaseLS(unsigned char& r, unsigned char& g, unsigned char& b, float l_factor, float s_factor);
