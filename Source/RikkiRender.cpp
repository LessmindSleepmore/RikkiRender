// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "../Header/Data/TgaImage.h"
#include "../Header/Data/OBJParser.h"
#include "../Header/Render/GraphicDrawing.h"
#include "../Header/Render/PostProcess.h"
#include "../Header/Data/Texture.h"
#include "../Header/Render/ToonRenderPipeline.h"
#include "../Header/Render/DepthRender.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor black = TGAColor(0, 0, 0, 255);
const TGAColor outerlinecolor = TGAColor(161, 103, 74, 255);
const TGAColor innerlinecolor = TGAColor(18, 18, 56, 255);
#define WIDHT 4096
#define HEIGHT 4096
#define PI 3.1425926535

void render(float lightxdegree) {

    OBJParser objfiles("Resource/mitadream_addFaceObj.obj");
    ToonRenderPipeline myrender(WIDHT, HEIGHT);
    DepthRender drender(WIDHT, HEIGHT);

    myrender.setObjFile(objfiles);
    drender.setObjFile(objfiles);

    // 模型变换矩阵
    Matrix rmat(0., 0., 0.);
    Matrix tmat(vec3f(0., 0, 0.));
    myrender.setModelTransform(rmat, tmat);
    drender.setModelTransform(rmat, tmat);

    // 设置相机
    vec3f cameraPos(0., 6., 3.); // 相机位置
    vec3f cameraRot(10., 180., 0.); // 旋转
    myrender.setCamera(cameraPos, cameraRot, 10, 0.1, 45, 1);
    drender.setCamera(cameraPos, cameraRot, 10, 0.1, 45, 1);

    // 设置光源
    myrender.setLightDirection(vec3f(cosf(lightxdegree), 0.0, sinf(lightxdegree))); // vec3f lightdir(0.8, 0.25, 1.0);
    //vec3f lightdir(camRotMat.MultipleVec3(vec3f(0., 0., -1.0))); // 设置一个相机发出的光源

    // 获取深度值
    drender.Commit();
    myrender.depthbufferVS = drender.getZBuffer();

    myrender.Commit();
    myrender.drawHairCastShadow(5);

    return;
}

int main()
{
    render(0.349177);

    return 0;
}