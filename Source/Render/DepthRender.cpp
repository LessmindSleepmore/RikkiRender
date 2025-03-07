#include "../../Header/Render/DepthRender.h"

DepthRender::DepthRender(int w, int h) : Render(w, h){}

void DepthRender::geometryVertexShader(int blockidx, int faceidx)
{
    face_info = objfiles.getFace(blockidx, faceidx);
    this->blockidx = blockidx;

    view_coords.clear();
    screen_coords.clear();

    for (int j = 0; j < 3; j++) {
        view_coords.push_back(viewmat.MultipleVec4(modelmat.MultipleVec4(vec4f(objfiles.getVert(face_info[j].x), 1.))));
        vec4f cilp_space_coord = projectionMat.MultipleVec4(view_coords[j]);

        // 齐次除法
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;

        screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));
    }
}

void DepthRender::clampInTriangle(vec3f _cv)
{

}

TGAColor DepthRender::fragmentShader()
{
    return TGAColor();
}

bool DepthRender::depthStencilTest(int _x, int _y)
{
    // 进行深度测试但不写入颜色缓冲
    if (zbuffer[_x * height + _y] > clampz) {
        zbuffer[_x * height + _y] = clampz;
    }
    return false;
}

float* DepthRender::getZBuffer()
{
    return zbuffer;
}
