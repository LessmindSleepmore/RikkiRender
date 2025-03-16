#pragma once
#include "../Data/TgaImage.h"
#include "../Data/OBJParser.h"
#include "../Math/CustomAlgorithm.h"
#include "../Data/Texture.h"
#include "PostProcess.h"

class Render {
    TGAImage image;
    TGAImage normalbuffer;
    OBJParser objfiles;

    int height, width;

    // MVP����
    Matrix modelmat;
    Matrix viewmat;
    Matrix projectionMat;

    std::vector<vec3i> face_info;

    std::vector<vec4f> world_coords;
    std::vector<vec3f> screen_coords;

    std::vector<vec3f> vertex_normals;
    std::vector<vec2f> vertex_uv;

    // �������
    vec3f cameraPos; // ���λ��
    vec3f cameraRot; // ��ת

    vec3f lightdir;
    int blockidx;

    float* zbuffer;
    unsigned char* stencilbuffer;

    bool enablestencilwrite = false;
    unsigned char stencilbuffervalue = 1;

    // ��߲���
    const TGAColor outerlinecolor = TGAColor(161, 103, 74, 255);
    const TGAColor innerlinecolor = TGAColor(18, 18, 56, 255);

public:
    // ��������
    vec3f facecenter;
    Texture ramptex;

public:
    Render(int width, int height);

    void stencilTestSettting(bool enableST, int stencilvalue);

    void setObjFile(OBJParser &f);

    void setModelTransform(Matrix rotation, Matrix location);

    void setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect);

    void setLightDirection(vec3f l);

    void commit();

private:
    vec3f calculateBarycentricCoordinates(const std::vector<vec3f>& screen_coords, int _x, int _y);

    void rasterize();

    TGAColor fragmentShader(vec3f worldpos, vec3f& worldnormal, vec2f uv);

    void geometryVertexShader(int blockidx, int faceidx);

    void beginPostProcess();

    void Draw();

    void Shut();
};