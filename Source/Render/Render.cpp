#include "../../Header/Render/Render.h"

Render::Render(int w, int h) : 
    image(w, h, TGAImage::RGB, vec4c(184, 216, 216, 255)),
    normalbuffer(w, h, TGAImage::RGB, vec4c(0, 0, 0, 255)),
    zbuffer(new float[w * h]),
    stencilbuffer(new unsigned char[w * h]),
    height(h),
    width(w),
    blockidx(0)
{
    // 深度缓冲默认值不能取太大值，否则在做边缘检测卷积的时候会出现溢出或精度问题.
    for (int i = 0; i < w * h; ++i) zbuffer[i] = 10000;
    // 初始化模板缓冲
    for (int i = 0; i < w * h; ++i) stencilbuffer[i] = 0;
    // 模型变换矩阵
}

void Render::stencilTestSettting(bool enableST, int stencilvalue)
{
    enablestencilwrite = enableST;
    stencilbuffervalue = stencilvalue;
}

void Render::setObjFile(OBJParser& f)
{
    objfiles = f;
}

void Render::setModelTransform(Matrix rotation, Matrix location)
{
    modelmat = location.MultipleMat(rotation);
}

vec3f Render::calculateBarycentricCoordinates(const std::vector<vec3f>& screen_coords, int _x, int _y) {
    vec3f _cv = cross(vec3f((screen_coords[1] - screen_coords[0]).x, (screen_coords[2] - screen_coords[0]).x, (screen_coords[0] - vec3f(_x, _y, 0)).x),
        vec3f((screen_coords[1] - screen_coords[0]).y, (screen_coords[2] - screen_coords[0]).y, (screen_coords[0] - vec3f(_x, _y, 0)).y));
    return _cv / _cv.z;
}

void Render::rasterize()
{
    // 判断是否丢弃片段
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

    for (int _x = (int)round(vertmin); _x <= vertmax; ++_x) {
        for (int _y = (int)round(horimin); _y <= horimax; ++_y) {
            // 计算质心坐标 (1 - u - v, u, v)
            vec3f _cv = calculateBarycentricCoordinates(screen_coords, _x, _y);
            // 判断是否在三角形内部
            if (_cv.x >= 0 && _cv.y >= 0 && _cv.x + _cv.y <= 1) {
                // 丢弃画面外的像素点
                if (_x >= width || _y >= height || _x < 0 || _y < 0) {
                    continue;
                }
                // 插值z
                float clampz = (1 - _cv.x - _cv.y) * screen_coords[0].z + _cv.x * screen_coords[1].z + _cv.y * screen_coords[2].z;

                // 插值法线
                vec3f clampnormal = normalize(vertex_normals[0] * (1 - _cv.x - _cv.y) + vertex_normals[1] * _cv.x + vertex_normals[2] * _cv.y);
                // 插值世界坐标
                vec3f clampWP = world_coords[0].xyz() * (1 - _cv.x - _cv.y) + world_coords[1].xyz() * _cv.x + world_coords[2].xyz() * _cv.y;
                // 插值uv
                vec2f clampUV = vertex_uv[0] * (1 - _cv.x - _cv.y) + vertex_uv[1] * _cv.x + vertex_uv[2] * _cv.y;

                TGAColor rescolor = fragmentShader(clampWP, clampnormal, clampUV);


                // 深度模板测试
                if (zbuffer[_x * height + _y] > clampz) {
                    zbuffer[_x * height + _y] = clampz;
                    image.set(_x, _y, rescolor);
                    normalbuffer.set(_x, _y, TGAColor(abs(clampnormal.x) * 255, abs(clampnormal.y) * 255, abs(clampnormal.z) * 255, 255));

                    if (enablestencilwrite) {
                        stencilbuffer[_x * height + _y] = stencilbuffervalue;
                    }
                    else {
                        // 模板缓冲设置为初始值
                        stencilbuffer[_x * height + _y] = 0;
                    }
                }
            }
        }
    }
}

void Render::setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect)
{
    this->cameraPos = cameraPos;
    this->cameraRot = cameraRot;
    projectionMat = Matrix::MakeProjectionMatrix(f, n, fov, aspect);
    viewmat = Matrix(cameraPos, cameraRot);
}

TGAColor Render::fragmentShader(vec3f worldpos, vec3f &worldnormal, vec2f uv)
{
    if (blockidx == 6) {
        // 如果是面部则利用中心点重新计算法线
        worldnormal = normalize(worldpos - facecenter);
    }

    //// 计算光照强度
    float ndotl = dot(worldnormal, normalize(lightdir));
    float ndotv = dot(worldnormal, normalize(cameraPos - worldpos));
    //ndotl = fmax(0, ndotl);  // 为实现面部的正确阴影ndotl需要在-1.0到1.0之间
    ndotv = fmax(0, ndotv);
    // 二维Ramp采样
    vec4f rampcolor = ramptex.samplerTexure(vec2f(ndotl, ndotv));
    //vec4f rampcolor(1.0, 1.0, 1., 1.);

    // 边缘光
    float fresnelvalue = 1. + pow(1. - ndotv, 7) / 2.; // ndotv效果不好



    vec4c color = objfiles.samplerTexture2D(objfiles.fromBlockIdx2TextureIdx(blockidx), uv);
    vec4f tempcolor = rampcolor * fresnelvalue;
    TGAColor rescolor = TGAColor(static_cast<unsigned char>(fmin(static_cast<float>(color.x) * tempcolor.x, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.y) * tempcolor.y, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.z) * tempcolor.z, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.w) * tempcolor.w, 255)));

    return rescolor;
}

void Render::setLightDirection(vec3f l)
{
    lightdir = l;
}

void Render::geometryVertexShader(int blockidx, int faceidx)
{
    face_info = objfiles.getFace(blockidx, faceidx);
    this->blockidx = blockidx;

    world_coords.clear();
    screen_coords.clear();
    vertex_normals.clear();
    vertex_uv.clear();

    for (int j = 0; j < 3; j++) {
        // 模型变换后不需要进行齐次除法
        world_coords.push_back(modelmat.MultipleVec4(vec4f(objfiles.getVert(face_info[j].x), 1.)));
        vec4f cilp_space_coord = projectionMat.MultipleVec4(viewmat.MultipleVec4(world_coords[j]));

        // 齐次除法
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;

        screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., - cilp_space_coord.z));

        // 添加对应的顶点法线(这里没变换到世界坐标，因为没移动旋转模型所以没问题)
        vertex_normals.push_back(objfiles.getNormals(face_info[j].z));

        // 添加对应的uv坐标
        vertex_uv.push_back(objfiles.getUV(face_info[j].y));
    }
}

void Render::commit()
{
    // 设置shader使用的变量
    facecenter = objfiles.getVert(objfiles.getFace(7, 0)[0].x);
    facecenter = modelmat.MultipleVec4(vec4f(facecenter, 1.0)).xyz();
    ramptex = Texture("Resource/Ramp2D.png");

    for (int blockidx = 0; blockidx < objfiles.nBlock(); ++blockidx) {

        // 设置面部写入模板值，不进行后处理描边
        if (blockidx == 6 || blockidx == 4) stencilTestSettting(true, 1);
        else stencilTestSettting(false, 1);

        for (int faceidx = 0; faceidx < objfiles.nFaces(blockidx); ++faceidx) {
            geometryVertexShader(blockidx, faceidx);

            rasterize();
        }
        std::cout << "Finished render block number: " << blockidx << std::endl;
    }

    beginPostProcess();

    Draw();
    Shut();
}

void Render::beginPostProcess()
{
    // 外描边后处理（这里用边缘检测实现）
    PostProcess::sobelEdgeDetection(zbuffer, stencilbuffer, normalbuffer, width, height, image, outerlinecolor, innerlinecolor, true, stencilbuffervalue);
    PostProcess::Flare(width, height, image);
    std::cout << "Finished PostProcess." << std::endl;
}

void Render::Draw()
{
    image.flip_vertically();
    image.write_tga_file("Resource/output.tga");
    normalbuffer.flip_vertically();
    normalbuffer.write_tga_file("Resource/NormalBuffer.tga");
}

void Render::Shut()
{
    delete[] zbuffer;
    delete[] stencilbuffer;
}
