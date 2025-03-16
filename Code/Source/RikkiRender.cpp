// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <chrono>
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
#define WIDHT 512
#define HEIGHT 512
#define PI 3.1425926535

void render(float lightxdegree) {

    OBJParser objfiles("Resource/mitadream_addFaceObj.obj");
    ToonRenderPipeline myrender(WIDHT, HEIGHT, AntiAliasing::MSAA, MSAA_Param::MSAAx2);

    myrender.setObjFile(objfiles);

    // 模型变换矩阵
    Matrix rmat(0., 0., 0.);
    Matrix tmat(vec3f(0., 0, 0.));
    myrender.setModelTransform(rmat, tmat);

    // 设置相机
    vec3f cameraPos(0., 6., 3.); // 相机位置
    vec3f cameraRot(10., 180., 0.); // 旋转
    myrender.setCamera(cameraPos, cameraRot, 10, 0.1, 45, 1);

    // 设置光源
    myrender.setLightDirection(vec3f(cosf(lightxdegree), 0.0, sinf(lightxdegree))); // vec3f lightdir(0.8, 0.25, 1.0);
    //vec3f lightdir(camRotMat.MultipleVec3(vec3f(0., 0., -1.0))); // 设置一个相机发出的光源

    // 计时开始
    auto start = std::chrono::high_resolution_clock::now();

    myrender.Commit();
    myrender.drawHairCastShadow(5);

    // 计时结束
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;

    std::cout << "myrender.Commit()耗时: " << elapsed.count() << " 毫秒" << std::endl;

    return;
}

int main()
{
    render(0.349177);

    return 0;
}