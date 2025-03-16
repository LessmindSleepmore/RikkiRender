#pragma once
#include "RenderPipeline.h"
#include "DepthRender.h"

enum AntiAliasing {
    NoAA,
    SSAA,
    MSAA
};

enum SSAA_Param {
    SSAAx2 = 2,
    SSAAx4 = 4,
    SSAAx8 = 8
};

enum MSAA_Param {
    MSAAx2 = 2,
    MSAAx4 = 4,
    MSAAx8 = 8
};

class ToonRenderPipeline : public RenderPipeline {

    TGAImage normalbuffer;
    // 描边参数
    const TGAColor outerlinecolor = TGAColor(161, 103, 74, 255);
    const TGAColor innerlinecolor = TGAColor(18, 18, 56, 255);

    // 边缘光
    DepthRender drender;

    //Clamp
    vec3f clampnormal;
    vec3f clampoffset;
    vec3f clampWP;
    vec2f clampUV;

    // 其他参数
    vec3f facecenter;
    Texture ramptex;
    std::vector<vec3f> offset_vertexCS;


    // 抗锯齿
    AntiAliasing aa;
    SSAA_Param ssaa_param;
    TGAImage ssaa_downsampler_img;

    MSAA_Param msaa_param;
    TGAImage colorMSAABuffer;  // 颜色缓存


public:
    // 视角空间下的深度数据
    float* depthbufferVS;
    float rimlightwidth;

    ToonRenderPipeline(int w, int h) : RenderPipeline(w, h),
        normalbuffer(w, h, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        depthbufferVS(nullptr),
        rimlightwidth(0.5),
        aa(AntiAliasing::NoAA),
        drender(w, h)
    {};

    ToonRenderPipeline(int w, int h, AntiAliasing _aa) : RenderPipeline(w, h),
        normalbuffer(w, h, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        depthbufferVS(nullptr),
        rimlightwidth(0.5),
        aa(_aa),
        drender(w, h)
    {
    };

    ToonRenderPipeline(int w, int h, AntiAliasing _aa, SSAA_Param _sp) : RenderPipeline(w * _sp, h * _sp),
        normalbuffer(w * _sp, h * _sp, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        ssaa_param(_sp),
        ssaa_downsampler_img(w, h, TGAImage::RGB, vec4c(184, 216, 216, 255)),
        depthbufferVS(nullptr),
        rimlightwidth(0.5),
        aa(_aa),
        drender(w * _sp, h * _sp)
    {};

    ToonRenderPipeline(int w, int h, AntiAliasing _aa, MSAA_Param _sp) : RenderPipeline(w * _sp, h * _sp),
        normalbuffer(w, h, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        msaa_param(_sp),
        colorMSAABuffer(w * _sp, h * _sp, TGAImage::RGB, vec4c(0, 0, 0, 255)),
        depthbufferVS(nullptr),
        rimlightwidth(0.5),
        aa(_aa),
        drender(w* _sp, h* _sp)
    {
    };

    void drawHairCastShadow(int bidx);

    void setObjFile(OBJParser& f) override;

    void setModelTransform(Matrix rotation, Matrix location) override;

    void setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect) override;


private:
    void clampInTriangle(vec3f _cv) override;

    void postProcess() override;

    TGAColor fragmentShader(int x, int y) override;

    void geometryVertexShader(int blockidx, int faceidx) override;

    void writeTexture(int _x, int _y, TGAColor rescolor) override;

    void Draw() override;

    void customDataSet() override;

    void Pipeline() override;

    void rasterize() override;

    void NoAARasterize();

    void MSAARasterize();

    void DownsampleSSAA();
};