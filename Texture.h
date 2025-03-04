#pragma once
#include "GeometricTypes.h"
#include <iostream>

class Texture {

	unsigned char* img;
	int width, height, channels;
public:
	Texture(const char* filename);
	~Texture();

	vec4f samplerTexure(vec2f uv);
};