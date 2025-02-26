#pragma once

#include "tgaimage.h"
#include "CustomAlgorithm.h"
#include "GeometricTypes.h"
#include <algorithm>
#include <vector>

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void triangle(vec2i v0, vec2i v1, vec2i v2, TGAImage& image, TGAColor color);

void rasterize(const std::vector<vec3f> scpos, TGAImage& image, TGAColor color, float* zbuffer, vec2i resolution);