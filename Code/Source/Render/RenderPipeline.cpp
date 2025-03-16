#include "../../Header/Render/RenderPipeline.h"

RenderPipeline::RenderPipeline(int w, int h) : 
    image(w, h, TGAImage::RGB, vec4c(184, 216, 216, 255)),
    zbuffer(new float[w * h]),
    stencilbuffer(new unsigned char[w * h]),
    height(h),
    width(w),
    blockidx(0),
    clampz(0)
{
    // 深度缓冲默认值不能取太大值，否则在做边缘检测卷积的时候会出现溢出或精度问题.
    for (int i = 0; i < w * h; ++i) zbuffer[i] = 10000;
    // 初始化模板缓冲
    for (int i = 0; i < w * h; ++i) stencilbuffer[i] = 0;
}

RenderPipeline::~RenderPipeline()
{
    delete[] zbuffer;
    delete[] stencilbuffer;
}

void RenderPipeline::stencilTestSettting(bool enableST, int stencilvalue)
{
    enablestencilwrite = enableST;
    stencilbuffervalue = stencilvalue;
}

void RenderPipeline::setObjFile(OBJParser& f)
{
    objfiles = f;
}

void RenderPipeline::setModelTransform(Matrix rotation, Matrix location)
{
    modelmat = location.MultipleMat(rotation);
}

void RenderPipeline::clampZ(vec3f _cv)
{
    clampz = (1 - _cv.x - _cv.y) * screen_coords[0].z + _cv.x * screen_coords[1].z + _cv.y * screen_coords[2].z;
}


// 计算重心坐标
vec3f RenderPipeline::calculateBarycentricCoordinates(const std::vector<vec3f>& screen_coords, float _x, float _y) {
    vec3f _cv = cross(vec3f((screen_coords[1] - screen_coords[0]).x, (screen_coords[2] - screen_coords[0]).x, (screen_coords[0] - vec3f(_x, _y, 0)).x),
        vec3f((screen_coords[1] - screen_coords[0]).y, (screen_coords[2] - screen_coords[0]).y, (screen_coords[0] - vec3f(_x, _y, 0)).y));
    return _cv / _cv.z;
}

void RenderPipeline::rasterize()
{
    // 丢弃片段屏幕外的面片
    bool isthrow = true;
    for (int i = 0; i < 3; ++i) {
        if (screen_coords[i].x < width && screen_coords[i].y < height) {
            isthrow = false;
            break;
        }
    }
    if (isthrow) return;

    // bounding box
    float vertmax = std::numeric_limits<int>::min();
    float vertmin = std::numeric_limits<int>::max();
    float horimax = std::numeric_limits<int>::min();
    float horimin = std::numeric_limits<int>::max();
    for (int i = 0; i < 3; ++i) {
        vertmax = screen_coords[i].x > vertmax ? screen_coords[i].x : vertmax;
        vertmin = screen_coords[i].x < vertmin ? screen_coords[i].x : vertmin;
        horimax = screen_coords[i].y > horimax ? screen_coords[i].y : horimax;
        horimin = screen_coords[i].y < horimin ? screen_coords[i].y : horimin;
    }

    for (_x = (int)round(vertmin); _x <= vertmax; ++_x) {
        for (_y = (int)round(horimin); _y <= horimax; ++_y) {
            // 计算质心坐标 (1 - u - v, u, v)
            vec3f _cv = calculateBarycentricCoordinates(screen_coords, _x, _y);
            // 判断是否在三角形内部
            if (_cv.x >= 0 && _cv.y >= 0 && _cv.x + _cv.y <= 1) {
                // 丢弃画面外的像素点
                if (_x >= width || _y >= height || _x < 0 || _y < 0) {
                    continue;
                }

                // 根据质心坐标插值
                clampZ(_cv);
                // 自定义插值
                clampInTriangle(_cv);

                // 片元着色器
                TGAColor rescolor = fragmentShader(_x, _y);

                // OM
                if (depthStencilTest(_x, _y)) {
                    writeTexture(_x, _y, rescolor);
                }
            }
        }
    }
}

void RenderPipeline::customDataSet()
{
}

void RenderPipeline::postProcess()
{
}

void RenderPipeline::Pipeline()
{
    for (int blockidx = 0; blockidx < objfiles.nBlock(); ++blockidx) {
        for (int faceidx = 0; faceidx < objfiles.nFaces(blockidx); ++faceidx) {
            geometryVertexShader(blockidx, faceidx);
            rasterize();
        }
        std::cout << "Finished render block number: " << blockidx << std::endl;
    }
}

void RenderPipeline::setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect)
{
    this->cameraPos = cameraPos;
    this->cameraRot = cameraRot;
    projectionMat = Matrix::MakeProjectionMatrix(f, n, fov, aspect);
    viewmat = Matrix(cameraPos, cameraRot);
}

void RenderPipeline::setLightDirection(vec3f l)
{
    lightdir = l;
}

void RenderPipeline::Commit()
{
    customDataSet();
    Pipeline();
    //postProcess();
    Draw();
    Shut();
}

void RenderPipeline::Draw()
{
    image.flip_vertically();
    image.write_tga_file("Resource/output.tga");
}

void RenderPipeline::Shut()
{

}

bool RenderPipeline::depthStencilTest(int _x, int _y)
{
    // 深度模板测试
    if (zbuffer[_x * height + _y] > clampz) {
        zbuffer[_x * height + _y] = clampz;
        return true;
    }
    return false;
}

void RenderPipeline::writeTexture(int _x, int _y, TGAColor rescolor)
{
    image.set(_x, _y, rescolor);
}
