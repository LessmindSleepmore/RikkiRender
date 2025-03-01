// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "tgaimage.h"
#include "OBJParser.h"
#include "GraphicDrawing.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
#define WIDHT 1080
#define HEIGHT 1080

int main()
{
	TGAImage image(WIDHT, HEIGHT, TGAImage::RGB);
    OBJParser objfiles("mita.obj");
    float *zbuffer = new float[WIDHT * HEIGHT];
    for (int i = 0; i < WIDHT * HEIGHT; ++i) zbuffer[i] = std::numeric_limits<float>::max();

    // 模型变换矩阵
    Matrix rmat(0., 0., 0.);
    Matrix tmat(vec3f(0., 0, 0.));
    Matrix modelmat = tmat.MultipleMat(rmat);

    // ViewMatrix
    vec3f cameraPos(0., 3.5, 5.); // 相机位置
    vec3f cameraRot(0., 180., 0.); // 旋转
    Matrix viewmat(cameraPos, cameraRot);
    Matrix camRotMat(cameraRot.x, cameraRot.y, cameraRot.z);

    //设置一个相机发出的光源
    vec3f lightdir(camRotMat.MultipleVec3(vec3f(0., 0., -1.0)));

    // projectionMatrix
    Matrix projectionMat = Matrix::MakeProjectionMatrix(10, 0.1, 45, 1);

    // 渲染模型测试用例
    for (int faceidx = 0; faceidx < objfiles.nFaces(); ++faceidx) {
        std::vector<vec3i> fi = objfiles.getFace(faceidx);
        std::vector<vec3f> screen_coords(3);
        std::vector<vec3f> vertex_normals;
        vec4f local_coords[3];
        vec4f Mmat_coords[3];
        vec4f MVmat_coords[3];
        vec4f MVPmat_coords[3];
        for (int j = 0; j < 3; j++) {
            local_coords[j] = vec4f(objfiles.getVert(fi[j].x), 1.);
            Mmat_coords[j] = modelmat.MultipleVec4(local_coords[j]);
            MVmat_coords[j] = viewmat.MultipleVec4(Mmat_coords[j]);
            MVPmat_coords[j] = projectionMat.MultipleVec4(MVmat_coords[j]);

            // 齐次除法
            MVPmat_coords[j].x /= MVPmat_coords[j].w;
            MVPmat_coords[j].y /= MVPmat_coords[j].w;

            screen_coords[j] = vec3f((MVPmat_coords[j].x + 1.) * (WIDHT - 1) / 2., (MVPmat_coords[j].y + 1.) * (HEIGHT - 1) / 2., -MVPmat_coords[j].z);

            // 添加对应的顶点法线(这里没变换到世界坐标，因为没移动旋转模型所以没问题)
            vertex_normals.push_back(objfiles.getNormals(fi[j].z));
        }
        //vec3f n = normalize(cross(normalize(Mmat_coords[1].xyz() - Mmat_coords[0].xyz()), normalize(Mmat_coords[2].xyz() - Mmat_coords[0].xyz())));
        //float ndotl = dot(n, lightdir);

        //triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(255 * ndotl, 255 * ndotl, 255 * ndotl, 255));
        rasterize(screen_coords, vertex_normals, image, TGAColor(255, 255, 255, 255), zbuffer, vec2i(WIDHT, HEIGHT), lightdir);

        if (faceidx % 100 == 0) std::cout << "Finished render faces number: " << faceidx << std::endl;
    }

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
	image.write_tga_file("output.tga");
	return 0;
}