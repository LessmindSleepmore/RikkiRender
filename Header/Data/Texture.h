#pragma once
#include "../Math/GeometricTypes.h"
#include <iostream>

class Texture {

	unsigned char* img;
	int width, height, channels;
public:
	Texture(const char* filename);
	Texture();
	~Texture();

	Texture& operator=(const Texture& t) {
		if (this != &t) {
			if (img) {
				delete[] img;
			}
			width = t.width;
			height = t.height;
			channels = t.channels;

			img = new unsigned char[width * height * 4];
			std::memcpy(img, t.img, width * height * 4);
		}

		return *this;
	}

	vec4f samplerTexure(vec2f uv);
};