#include "../../Header/Render/PostProcess.h"

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

    // 内边缘 Sobel 算子
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

    // 外边缘 Sobel 算子
    std::vector<vec2i> Gxoffsets = { vec2i(-2, -2), vec2i(0, -2), vec2i(2, -2), vec2i(-2, 2), vec2i(0, 2), vec2i(2, 2) };  // 偏移量，可根据实际卷积核调整
    std::vector<vec2i> Gyoffsets = { vec2i(-2, -2), vec2i(-2, 0), vec2i(-2, 2), vec2i(2, -2), vec2i(2, 0), vec2i(2, 2) };
    std::vector<int> values = { -1, -2, -1, 1, 2, 1 };  // 对应的值，可根据实际卷积核调整

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
    for (int y = 2; y < height - 2; ++y) {
        for (int x = 2; x < width - 2; ++x) {
            float gx = 0.0f;
            float gy = 0.0f;

            for (size_t k = 0; k < Gxoffsets.size(); ++k) {
                int offsetX = Gxoffsets[k].x;
                int offsetY = Gxoffsets[k].y;
                int index = (y + offsetY) * width + (x + offsetX);
                gx += zbuffer[index] * values[k];
            }
            for (size_t k = 0; k < Gyoffsets.size(); ++k) {
                int offsetX = Gyoffsets[k].x;
                int offsetY = Gyoffsets[k].y;
                int index = (y + offsetY) * width + (x + offsetX);
                gy += zbuffer[index] * values[k];
            }
            float gradientMagnitude = std::fmin(std::sqrt(gx * gx + gy * gy) - 100., 1.0);
            if (gradientMagnitude > 0.001) {
                img.set(y, x, outercolor);
            }
        }
    }
    return;
}

void PostProcess::Flare(int width, int height, TGAImage& img) {
    TGAColor backgroundcolor = TGAColor(255, 218, 148, 255);

    for (int y = 0; y < height; ++y) {
        float y_intensity = static_cast<float>(y) / static_cast<float>(height);
        y_intensity = pow(y_intensity, 2);

        for (int x = 0; x < width; ++x) {
            float x_intensity = (width - static_cast<float>(x)) / static_cast<float>(width);
            x_intensity = pow(x_intensity, 2);

            float intensity = x_intensity * y_intensity;

            TGAColor originalcolor = img.get(x, y);
            int r = static_cast<int>(originalcolor.r * (1 - intensity) + backgroundcolor.r * intensity);
            int g = static_cast<int>(originalcolor.g * (1 - intensity) + backgroundcolor.g * intensity);
            int b = static_cast<int>(originalcolor.b * (1 - intensity) + backgroundcolor.b * intensity);
            int a = static_cast<int>(originalcolor.a * (1 - intensity) + backgroundcolor.a * intensity);

            TGAColor newColor = TGAColor(r, g, b, a);
            img.set(x, y, newColor);
        }
    }
}
