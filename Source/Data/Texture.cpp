#include "../../Header/Data/Texture.h"
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

Texture::Texture(): img(nullptr), width(0), height(0), channels(0){}

Texture::~Texture()
{
	if (img) {
		stbi_image_free(img);
	}
}

vec4f Texture::samplerTexure(vec2f uv)
{
	uv.x = fmin(fmax(uv.x, 0.), 1.);
	uv.y = fmin(fmax(uv.y, 0.), 1.);
	int x = static_cast<int>((uv.x + 1.) / 2 * (width - 1));
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
