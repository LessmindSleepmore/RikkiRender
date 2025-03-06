#pragma once

#include "../Data/TgaImage.h"
#include "../Math/CustomAlgorithm.h"
#include "../Data/Texture.h"
#include "../Math/GeometricTypes.h"
#include "../Data/OBJParser.h"
#include <algorithm>
#include <vector>

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

void triangle(vec2i v0, vec2i v1, vec2i v2, TGAImage& image, TGAColor color);

void rasterize(std::vector<vec3f> scpos,
    std::vector<vec3f> wspos,
    std::vector<vec3f> vertex_normals,
    std::vector<vec2f> vertex_uv,
    TGAImage& image,
    TGAImage& normalbuffer,
    OBJParser& objparser,
    int textureIdx,
    TGAColor defualtcolor,
    float* zbuffer,
    unsigned char* stencilbuffer,
    bool enablestencilbuffer,
    unsigned stencilbuffervalue,
    vec2i resolution,
    vec3f lightdir,
    vec3f cameraPos,
    Texture& ramptex,
    int blockidx,
    vec3f faceCenter);