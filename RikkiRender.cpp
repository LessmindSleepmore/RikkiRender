// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "tgaimage.h"
#include "OBJParser.h"
#include "GraphicDrawing.h"
#include "PostProcess.h"
#include "Texture.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor black = TGAColor(0, 0, 0, 255);
const TGAColor outerlinecolor = TGAColor(161, 103, 74, 255);
const TGAColor innerlinecolor = TGAColor(18, 18, 56, 255);
#define WIDHT 4096
#define HEIGHT 4096

int main()
{
	TGAImage image(WIDHT, HEIGHT, TGAImage::RGB, vec4c(184, 216, 216, 255));
    TGAImage normalbuffer(WIDHT, HEIGHT, TGAImage::RGB, vec4c(0, 0, 0, 255));
    OBJParser objfiles("Resource/mitadream_addFaceObj.obj");
    float *zbuffer = new float[WIDHT * HEIGHT];
    unsigned char *stencilbuffer = new unsigned char[WIDHT * HEIGHT];
    //for (int i = 0; i < WIDHT * HEIGHT; ++i) zbuffer[i] = sqrt(std::numeric_limits<float>::max());
    // 深度缓冲默认值不能取太大值，否则在做边缘检测卷积的时候会出现溢出或精度问题.
    for (int i = 0; i < WIDHT * HEIGHT; ++i) zbuffer[i] = 10000;
    // 初始化模板缓冲
    for (int i = 0; i < WIDHT * HEIGHT; ++i) stencilbuffer[i] = 0;
    // 模型变换矩阵
    Matrix rmat(0., 0., 0.);
    Matrix tmat(vec3f(0., 0, 0.));
    Matrix modelmat = tmat.MultipleMat(rmat);

    // ViewMatrix
    vec3f cameraPos(0., 6., 3.); // 相机位置
    vec3f cameraRot(10., 180., 0.); // 旋转
    Matrix viewmat(cameraPos, cameraRot);
    Matrix camRotMat(cameraRot.x, cameraRot.y, cameraRot.z);

    // 设置光源
    //vec3f lightdir(camRotMat.MultipleVec3(vec3f(0., 0., -1.0))); // 设置一个相机发出的光源
    vec3f lightdir(1.0, 0.0, 1.0); // vec3f lightdir(0.8, 0.25, 1.0);

    // projectionMatrix
    Matrix projectionMat = Matrix::MakeProjectionMatrix(10, 0.1, 45, 1);
    Texture ramptex("Resource/Ramp2D.png");

    // 获取面部网格中心
    vec3f facecenter = objfiles.getVert(objfiles.getFace(7, 0)[0].x);

    bool enablestencil = false;
    unsigned char stencilbuffervalue = 1;
    // 渲染模型测试用例
    for (int blockidx = 0; blockidx < objfiles.nBlock(); ++blockidx) {

        if (blockidx == 6 || blockidx == 4) enablestencil = true;
        else enablestencil = false;

        for (int faceidx = 0; faceidx < objfiles.nFaces(blockidx); ++faceidx) {
            std::vector<vec3i> fi = objfiles.getFace(blockidx, faceidx);
            std::vector<vec3f> screen_coords(3);
            std::vector<vec3f> world_coords(3);
            std::vector<vec3f> vertex_normals;
            std::vector<vec2f> vertex_uv;
            vec4f local_coords[3];
            vec4f Mmat_coords[3];
            vec4f MVmat_coords[3];
            vec4f MVPmat_coords[3];
            for (int j = 0; j < 3; j++) {
                local_coords[j] = vec4f(objfiles.getVert(fi[j].x), 1.);
                Mmat_coords[j] = modelmat.MultipleVec4(local_coords[j]);
                world_coords[j] = Mmat_coords[j].xyz() / Mmat_coords[j].w;
                MVmat_coords[j] = viewmat.MultipleVec4(Mmat_coords[j]);
                MVPmat_coords[j] = projectionMat.MultipleVec4(MVmat_coords[j]);

                // 齐次除法
                MVPmat_coords[j].x /= MVPmat_coords[j].w;
                MVPmat_coords[j].y /= MVPmat_coords[j].w;

                screen_coords[j] = vec3f((MVPmat_coords[j].x + 1.) * (WIDHT - 1) / 2., (MVPmat_coords[j].y + 1.) * (HEIGHT - 1) / 2., -MVPmat_coords[j].z);

                // 添加对应的顶点法线(这里没变换到世界坐标，因为没移动旋转模型所以没问题)
                vertex_normals.push_back(objfiles.getNormals(fi[j].z));

                // 添加对应的uv坐标
                vertex_uv.push_back(objfiles.getUV(fi[j].y));
            }
            //vec3f n = normalize(cross(normalize(Mmat_coords[1].xyz() - Mmat_coords[0].xyz()), normalize(Mmat_coords[2].xyz() - Mmat_coords[0].xyz())));
            //float ndotl = dot(n, lightdir);

            //triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(255 * ndotl, 255 * ndotl, 255 * ndotl, 255));
            rasterize(screen_coords,
                world_coords,
                vertex_normals,
                vertex_uv,
                image,
                normalbuffer,
                objfiles,
                objfiles.fromBlockIdx2TextureIdx(blockidx),
                red, 
                zbuffer,
                stencilbuffer,
                enablestencil,
                stencilbuffervalue,
                vec2i(WIDHT, HEIGHT), 
                lightdir,
                cameraPos,
                ramptex,
                blockidx,
                facecenter);
        }
        std::cout << "Finished render block number: " << blockidx << std::endl;
    }

    // 外描边后处理（这里用边缘检测实现）
    PostProcess::sobelEdgeDetection(zbuffer, stencilbuffer, normalbuffer, WIDHT, HEIGHT, image, outerlinecolor, innerlinecolor, true, stencilbuffervalue);
    PostProcess::Flare(WIDHT, HEIGHT, image);
    std::cout << "Finished outline draw." << std::endl;

    //// 渲染网格测试用例
    //for (int faceidx = 0; faceidx < objfiles.nFaces(); ++faceidx) {
    //    vec3i fi = objfiles.getFace(faceidx);
    //    for (int i = 0; i < 3; ++i) {
    //        vec3f v0 = objfiles.getVert(fi.raw[i]);
    //        vec3f v1 = objfiles.getVert(fi.raw[(i + 1) % 3]);
    //        int x0 = (v0.x + 1.) * WIDHT / 2;
    //        int x1 = (v1.x + 1.) * WIDHT / 2;
    //        int y0 = (v0.y + 1.) * HEIGHT / 2;
    //        int y1 = (v1.y + 1.) * HEIGHT / 2;
    //        line(x0, y0, x1, y1, image, white);
    //    }
    //}

    // 绘制三角形测试用例
    //std::vector<vec3f> t0 = { vec3f(10, 70, 0),   vec3f(50, 160, 0),  vec3f(70, 80, 0) };
    //vec2i t1[3] = { vec2i(180, 50),  vec2i(150, 1),   vec2i(70, 180) };
    //vec2i t2[3] = { vec2i(180, 150), vec2i(120, 160), vec2i(130, 180) };

    //rasterize(t0, image, white);
    //rasterize(t1[0], t1[1], t1[2], image, white);
    //rasterize(t2[0], t2[1], t2[2], image, white);

    delete[] zbuffer;
    image.flip_vertically();
	image.write_tga_file("Resource/output.tga");
    normalbuffer.flip_vertically();
    normalbuffer.write_tga_file("Resource/NormalBuffer.tga");
	return 0;
}