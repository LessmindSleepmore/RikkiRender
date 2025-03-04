#include "Texture.h"
#include "stb_image.h"

Texture::Texture(const char* filename)
{
	img = stbi_load(filename, &width, &height, &channels, 0);
	if (img) {
		std::cout << "Successfully loaded texture " << filename << std::endl;
	}
	else {
		std::cout << "Failed loaded texture " << filename << std::endl;
	}
}

Texture::~Texture()
{
	stbi_image_free(img);
}

vec4f Texture::samplerTexure(vec2f uv)
{
	int x = static_cast<int>(uv.x * (width - 1));
	int y = static_cast<int>((1. - uv.y) * (height - 1));

	// RGBA
	if (channels == 4) {
		int index = (y * width + x) * channels;
		unsigned char r = img[index];
		unsigned char g = img[index + 1];
		unsigned char b = img[index + 2];
		unsigned char a = img[index + 3];

		return vec4f(static_cast<float>(r) / 255., static_cast<float>(g) / 255., static_cast<float>(b) / 255., static_cast<float>(a) / 255.);
	}
	else if(channels == 3){
		int index = (y * width + x) * channels;
		unsigned char r = img[index];
		unsigned char g = img[index + 1];
		unsigned char b = img[index + 2];

		return vec4f(static_cast<float>(r) / 255., static_cast<float>(g) / 255., static_cast<float>(b) / 255., 1.0);
	}

	return vec4f();
}
