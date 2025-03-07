#pragma once
#include "../Data/TgaImage.h"
#include "../Data/OBJParser.h"
#include "../Math/CustomAlgorithm.h"
#include "../Data/Texture.h"
#include "PostProcess.h"

class Render {

protected:
    TGAImage image;
    OBJParser objfiles;

    int height, width;

    // MVP矩阵
    Matrix modelmat;
    Matrix viewmat;
    Matrix projectionMat;

    std::vector<vec3i> face_info;

    std::vector<vec4f> world_coords;
    std::vector<vec3f> screen_coords;

    std::vector<vec3f> vertex_normals;
    std::vector<vec2f> vertex_uv;

    // 相机参数
    vec3f cameraPos; // 相机位置
    vec3f cameraRot; // 旋转

    vec3f lightdir;
    int blockidx;

    int _x;
    int _y;

    float* zbuffer;
    unsigned char* stencilbuffer;

    bool enablestencilwrite = false;
    unsigned char stencilbuffervalue = 1;

    float clampz;

public:
    Render(int width, int height);
    ~Render();

    void stencilTestSettting(bool enableST, int stencilvalue);

    void setObjFile(OBJParser &f);

    void setModelTransform(Matrix rotation, Matrix location);

    void setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect);

    void setLightDirection(vec3f l);

    virtual void Commit();

    virtual void clampInTriangle(vec3f _cv) = 0;

    void clampZ(vec3f _cv);

protected:
    vec3f calculateBarycentricCoordinates(const std::vector<vec3f>& screen_coords, int _x, int _y);

    virtual void rasterize();

    virtual void customDataSet();

    virtual void postProcess();

    virtual void Pipeline();

    virtual TGAColor fragmentShader() = 0;

    virtual void geometryVertexShader(int blockidx, int faceidx) = 0;

    virtual void Draw();

    void Shut();

    virtual bool depthStencilTest(int _x, int _y);

    virtual void writeTexture(int _x, int _y, TGAColor rescolor);
};