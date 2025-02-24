// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "tgaimage.h"
#include "OBJParser.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool isSwap = false;
    if (x1 < x0) std::swap(x1, x0);
    if (y1 < y0) std::swap(y1, y0);
    if (abs(x1 - x0) < abs(y1 - y0)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
        isSwap = true;
    }
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1. - t) + y1 * t;
		if (isSwap) image.set(y, x, color);
		else image.set(x, y, color);
    }
}

int main()
{
	TGAImage image(100, 100, TGAImage::RGB);
    OBJParser objfiles("MitaDream.obj");
	image.write_tga_file("output.tga");
	return 0;
}