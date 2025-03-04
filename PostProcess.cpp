#include "PostProcess.h"

void PostProcess::sobelEdgeDetection(
    const float* zbuffer,
    const unsigned char* stencilbuffer,
    TGAImage& normalbufffer,
    int width,
    int height,
    TGAImage& img,
    TGAColor outercolor,
    TGAColor innercolor,
    bool enablestencil,
    unsigned char notequalstencilvalue)
{
    std::vector<float> edges(width * height, 0.0f);

    // Sobel 算子
    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    // 法线颜色缓冲边缘检测(内边缘)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            for (int c = 0; c < 3; ++c) {
                int gx = 0, gy = 0;
                for (int j = -1; j <= 1; ++j) {
                    for (int i = -1; i <= 1; ++i) {
                        int pixelValue = normalbufffer.getPixel(x + i, y + j, c);
                        gx += pixelValue * Gx[j + 1][i + 1];
                        gy += pixelValue * Gy[j + 1][i + 1];
                    }
                }
                int magnitude = static_cast<int>(std::sqrt(gx * gx + gy * gy));

                magnitude = std::min(255, std::max(0, magnitude)) - 244;

                // 实现颜色混合
                if (magnitude > 0.001 && stencilbuffer[x * height + y] != notequalstencilvalue) {
                    img.set(x, y, TGAColor(innercolor.r, innercolor.g, innercolor.b, innercolor.a));
                }
            }
        }
    }

    // 深度图边缘检测(外边缘)
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            float gx = 0.0f;
            float gy = 0.0f;

            for (int j = -1; j <= 1; ++j) {
                for (int i = -1; i <= 1; ++i) {
                    int index = (y + j) * width + (x + i);
                    gx += zbuffer[index] * Gx[j + 1][i + 1];
                    gy += zbuffer[index] * Gy[j + 1][i + 1];
                }
            }
            float gradientMagnitude = fmin(sqrt(gx * gx + gy * gy) - 100., 1.0);
            if (gradientMagnitude > 0.001) {
                img.set(y, x, TGAColor(outercolor.r, outercolor.g, outercolor.b, outercolor.a));
            }
        }
    }

    return;
}
