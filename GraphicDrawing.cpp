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

/**
* 
* @param scpos: [float]The range of X and Y is [0, width] and [0, height] respectively.
*/
void rasterize(const std::vector<vec3f> scpos, TGAImage& image, TGAColor color, float* zbuffer, vec2i resolution)
{
    // find bounding box
    float vertmax = std::numeric_limits<int>::min();
    float vertmin = std::numeric_limits<int>::max();
    float horimax = std::numeric_limits<int>::min();
    float horimin = std::numeric_limits<int>::max();
    for (int i = 0; i < 3; ++i) {
        vertmax = scpos[i].x > vertmax ? scpos[i].x : vertmax;
        vertmin = scpos[i].x < vertmin ? scpos[i].x : vertmin;
        horimax = scpos[i].y > horimax ? scpos[i].y : horimax;
        horimin = scpos[i].y < horimin ? scpos[i].y : horimin;
    }
    for (int _x = (int)round(vertmin); _x <= vertmax; ++_x) {
        for (int _y = (int)round(horimin); _y <= horimax; ++_y) {
            // 计算质心坐标 (1 - u - v, u, v)
            vec3f _cv = cross(vec3f((scpos[1] - scpos[0]).x, (scpos[2] - scpos[0]).x, (scpos[0] - vec3f(_x, _y, 0)).x),
                            vec3f((scpos[1] - scpos[0]).y, (scpos[2] - scpos[0]).y, (scpos[0] - vec3f(_x, _y, 0)).y));
            _cv = _cv / _cv.z;

            // 判断是否在三角形内部
            if (_cv.x >= 0 && _cv.y >= 0 && _cv.x + _cv.y <= 1) {
                float clampz = (1 - _cv.x - _cv.y) * scpos[0].z + _cv.x * scpos[1].z + _cv.y * scpos[2].z;
                if (zbuffer[_x * resolution.y + _y] < clampz) {
                    zbuffer[_x * resolution.y + _y] = clampz;
                    image.set(_x, _y, color);
                }
            }
        }
    }
}
