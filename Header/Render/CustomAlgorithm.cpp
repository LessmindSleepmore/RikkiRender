#include "../../Header/Math/CustomAlgorithm.h"

void RGB2HLS(unsigned char r, unsigned char g, unsigned char b, float& h, float& l, float& s)
{
    float r_f = r / 255.0f;
    float g_f = g / 255.0f;
    float b_f = b / 255.0f;

    float max_val = std::max({ r_f, g_f, b_f });
    float min_val = std::min({ r_f, g_f, b_f });

    l = (max_val + min_val) / 2.0f;

    if (max_val == min_val) {
        s = 0;
        h = 0;
    }
    else {
        float d = max_val - min_val;
        s = (l > 0.5f) ? d / (2.0f - max_val - min_val) : d / (max_val + min_val);

        if (max_val == r_f) {
            h = (g_f - b_f) / d + ((g_f < b_f) ? 6.0f : 0.0f);
        }
        else if (max_val == g_f) {
            h = (b_f - r_f) / d + 2.0f;
        }
        else {
            h = (r_f - g_f) / d + 4.0f;
        }
        h /= 6.0f;
    }
}

void HLS2RGB(float h, float l, float s, unsigned char& r, unsigned char& g, unsigned char& b) {
    float r_f, g_f, b_f;

    if (s == 0) {
        r_f = g_f = b_f = l;
    }
    else {
        auto hue2rgb = [](float p, float q, float t) {
            if (t < 0) t += 1.0f;
            if (t > 1) t -= 1.0f;
            if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
            if (t < 1.0f / 2.0f) return q;
            if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
            };

        float q = (l < 0.5f) ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;

        r_f = hue2rgb(p, q, h + 1.0f / 3.0f);
        g_f = hue2rgb(p, q, h);
        b_f = hue2rgb(p, q, h - 1.0f / 3.0f);
    }

    r = static_cast<unsigned char>(r_f * 255.0f);
    g = static_cast<unsigned char>(g_f * 255.0f);
    b = static_cast<unsigned char>(b_f * 255.0f);
}

void decreaseLS(unsigned char& r, unsigned char& g, unsigned char& b, float l_factor, float s_factor)
{
    float* hls = new float[3];
    RGB2HLS(r, g, b, hls[0], hls[1], hls[2]);

    hls[1] *= l_factor;
    hls[2] *= s_factor;

    HLS2RGB(hls[0], hls[1], hls[2], r, g, b);
}
