﻿// RikkiRender.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "tgaimage.h"
#include "OBJParser.h"
#include "GraphicDrawing.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
#define WIDHT 1024
#define HEIGHT 1024

int main()
{
	TGAImage image(WIDHT, HEIGHT, TGAImage::RGB);
    OBJParser objfiles("Sample.obj");

    // 渲染模型测试用例
    for (int faceidx = 0; faceidx < objfiles.nFaces(); ++faceidx) {
        vec3i fi = objfiles.getFace(faceidx);
        vec2i screen_coords[3];
        vec3f world_coords[3];
        for (int j = 0; j < 3; j++) {
            world_coords[j] = objfiles.getVert(fi.raw[j]);
            screen_coords[j] = vec2i((world_coords[j].x + 1.) * WIDHT / 2., (world_coords[j].y + 1.) * WIDHT / 2.);
        }
        vec3f n = normalize(cross(normalize(world_coords[1] - world_coords[0]), normalize(world_coords[2] - world_coords[0])));
        float ndotl = dot(n, vec3f(0, 0, 1));

        if (ndotl > 0) {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(255 * ndotl, 255 * ndotl, 255 * ndotl, 255));
        }
        if (faceidx % 100 == 0) std::cout << "Finished render faces number: " << faceidx << std::endl;
    }

    // 渲染网格测试用例
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
    //vec2i t0[3] = { vec2i(10, 70),   vec2i(50, 160),  vec2i(70, 80) };
    //vec2i t1[3] = { vec2i(180, 50),  vec2i(150, 1),   vec2i(70, 180) };
    //vec2i t2[3] = { vec2i(180, 150), vec2i(120, 160), vec2i(130, 180) };

    //triangle(t0[0], t0[1], t0[2], image, white);
    //triangle(t1[0], t1[1], t1[2], image, white);
    //triangle(t2[0], t2[1], t2[2], image, white);

    image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}