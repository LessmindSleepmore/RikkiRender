#pragma once
#include "Render.h"

class ToonRender : public Render {

    TGAImage normalbuffer;
    // 描边参数
    const TGAColor outerlinecolor = TGAColor(161, 103, 74, 255);
    const TGAColor innerlinecolor = TGAColor(18, 18, 56, 255);

    //Clamp
    vec3f clampnormal;
    vec3f clampoffset;
    vec3f clampWP;
    vec2f clampUV;

    // 其他参数
    vec3f facecenter;
    Texture ramptex;
    std::vector<vec3f> offset_vertexCS;

public:
    // 视角空间下的深度数据
    float* depthbufferVS;
    float rimlightwidth;

    ToonRender(int w, int h) : Render(w, h),
        normalbuffer(w, h, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        depthbufferVS(nullptr),
        rimlightwidth(0.5)
    {};

private:
    void clampInTriangle(vec3f _cv) override;

    void postProcess() override;

    TGAColor fragmentShader() override;

    void geometryVertexShader(int blockidx, int faceidx) override;

    void writeTexture(int _x, int _y, TGAColor rescolor) override;

    void Draw() override;

    void customDataSet() override;

    void Pipeline() override;
};