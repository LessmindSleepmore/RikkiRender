#pragma once
#include <vector>
#include "tgaimage.h"

class PostProcess {
public:
    static void sobelEdgeDetection(
        const float* zbuffer,
        const unsigned char* stencilbuffer,
        TGAImage& normalbufffer,
        int width,
        int height,
        TGAImage& img,
        TGAColor outercolor,
        TGAColor innercolor,
        bool enablestencil,
        unsigned char notequalstencilvalue);

    static void Flare(
        int width,
        int height, 
        TGAImage& img);
};