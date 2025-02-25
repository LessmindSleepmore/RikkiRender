#include "GraphicDrawing.h"

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool isSwap = false;
    if (abs(x1 - x0) < abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        isSwap = true;
    }
    if (x1 < x0) {
        std::swap(x1, x0);
        std::swap(y1, y0);
    }
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = round(y0 * (1. - t) + y1 * t); // round equal (int)(x + 0.5)
        if (isSwap) image.set(y, x, color);
        else image.set(x, y, color);
    }
}

void triangle(vec2i v0, vec2i v1, vec2i v2, TGAImage& image, TGAColor color)
{
    // sort y
    if (v1.y < v2.y) swapVec2(v1, v2);
    if (v0.y < v1.y) swapVec2(v0, v1);
    if (v1.y < v2.y) swapVec2(v1, v2);
    for (int i = 0; i <= (v0.y - v2.y); ++i) {
        int y_axis = v0.y - i;
        if (i < (v0.y - v1.y)) {
            int xleft = v0.x + (float)i / (float)(v0.y - v2.y + 0.001) * (v2.x - v0.x);
            int xright = v0.x + (float)i / (float)(v0.y - v1.y + 0.001) * (v1.x - v0.x);
            line(xleft, y_axis, xright, y_axis, image, color);
        }
        else {
            int xleft = v0.x + (float)i / (float)(v0.y - v2.y + 0.001) * (v2.x - v0.x);
            int xright = v1.x + (float)(i - (v0.y - v1.y)) / (float)(v1.y - v2.y + 0.001) * (v2.x - v1.x);
            line(xleft, y_axis, xright, y_axis, image, color);
        }
    }

    line(v0.x, v0.y, v1.x, v1.y, image, color);
    line(v1.x, v1.y, v2.x, v2.y, image, color);
    line(v2.x, v2.y, v0.x, v0.y, image, color);
}
