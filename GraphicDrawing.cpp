#include "GraphicDrawing.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor black = TGAColor(0, 0, 0, 255);

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
* @param scpos: [float]The range of X and Y is [0, width] and [0, height] respectively.
*/
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
    vec3f faceCenter)
{
    // �ж��Ƿ���Ƭ��
    bool isthrow = true;
    for (int i = 0; i < 3; ++i) {
        if (scpos[i].x < resolution.x && scpos[i].y < resolution.y) {
            isthrow = false;
            break;
        }
    }
    if (isthrow) return;


    // ���������ĵ�
    if (blockidx == 7) {
        image.set(scpos[0].x, scpos[0].y, red);
        return;
    }

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
            // ������������ (1 - u - v, u, v)
            vec3f _cv = cross(vec3f((scpos[1] - scpos[0]).x, (scpos[2] - scpos[0]).x, (scpos[0] - vec3f(_x, _y, 0)).x),
                            vec3f((scpos[1] - scpos[0]).y, (scpos[2] - scpos[0]).y, (scpos[0] - vec3f(_x, _y, 0)).y));
            _cv = _cv / _cv.z;
            // �ж��Ƿ����������ڲ�
            if (_cv.x >= 0 && _cv.y >= 0 && _cv.x + _cv.y <= 1) {
                // ��������������ص�
                if (_x >= resolution.x || _y >= resolution.y || _x < 0 || _y < 0) {
                    continue;
                }
                // ��ֵz
                float clampz = (1 - _cv.x - _cv.y) * scpos[0].z + _cv.x * scpos[1].z + _cv.y * scpos[2].z;

                if (blockidx == 6) {
                    // ������沿���������ĵ����¼��㷨��
                    for (int i = 0; i < 3; ++i) {
                        vertex_normals[i] = normalize(wspos[i] - faceCenter);
                    }
                }
                
                // ��ֵ����
                vec3f clampnormal = normalize(vertex_normals[0] * (1 - _cv.x - _cv.y) + vertex_normals[1] * _cv.x + vertex_normals[2] * _cv.y);
                // ��ֵ��������
                vec3f clampWP = normalize(scpos[0] * (1 - _cv.x - _cv.y) + scpos[1] * _cv.x + scpos[2] * _cv.y);

                // �������ǿ��
                float ndotl = dot(clampnormal, normalize(lightdir));
                float ndotv = dot(clampnormal, normalize(cameraPos - clampWP));
                //ndotl = fmax(0, ndotl);  // Ϊʵ���沿����ȷ��Ӱndotl��Ҫ��-1.0��1.0֮��
                ndotv = fmax(0, ndotv);
                // ��άRamp����
                vec4f rampcolor = ramptex.samplerTexure(vec2f(ndotl, ndotv));

                // ��Ե��
                float fresnelvalue =  1. + pow(1. - ndotv, 100) / 2.;

                // ��ֵuv
                vec2f clampUV = vertex_uv[0] * (1 - _cv.x - _cv.y) + vertex_uv[1] * _cv.x + vertex_uv[2] * _cv.y;
                vec4c color = objparser.samplerTexture2D(textureIdx, clampUV);
                vec4f tempcolor = rampcolor * fresnelvalue;
                TGAColor rescolor = TGAColor(static_cast<unsigned char>(fmin(static_cast<float>(color.x) * tempcolor.x, 255)),
                    static_cast<unsigned char>(fmin(static_cast<float>(color.y) * tempcolor.y, 255)),
                    static_cast<unsigned char>(fmin(static_cast<float>(color.z) * tempcolor.z, 255)),
                    static_cast<unsigned char>(fmin(static_cast<float>(color.w) * tempcolor.w, 255)));

                if (zbuffer[_x * resolution.y + _y] > clampz) {
                    zbuffer[_x * resolution.y + _y] = clampz;
                    image.set(_x, _y, rescolor);
                    normalbuffer.set(_x, _y, TGAColor(abs(clampnormal.x) * 255, abs(clampnormal.y) * 255, abs(clampnormal.z) * 255, 255));

                    if (enablestencilbuffer) {
                        stencilbuffer[_x * resolution.y + _y] = stencilbuffervalue;
                    }
                    else {
                        // ģ�建������Ϊ��ʼֵ
                        stencilbuffer[_x * resolution.y + _y] = 0;
                    }
                }
            }
        }
    }
}