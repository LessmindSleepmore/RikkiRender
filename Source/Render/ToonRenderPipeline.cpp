#include "../../Header/Render/ToonRenderPipeline.h"

void ToonRenderPipeline::clampInTriangle(vec3f _cv)
{
    clampnormal = normalize(vertex_normals[0] * (1 - _cv.x - _cv.y) + vertex_normals[1] * _cv.x + vertex_normals[2] * _cv.y);
    clampWP = world_coords[0].xyz() * (1 - _cv.x - _cv.y) + world_coords[1].xyz() * _cv.x + world_coords[2].xyz() * _cv.y;
    clampUV = vertex_uv[0] * (1 - _cv.x - _cv.y) + vertex_uv[1] * _cv.x + vertex_uv[2] * _cv.y;
    clampoffset = offset_vertexCS[0] * (1 - _cv.x - _cv.y) + offset_vertexCS[1] * _cv.x + offset_vertexCS[2] * _cv.y;
}

void ToonRenderPipeline::postProcess()
{
    // 外描边后处理（这里用边缘检测实现）
    PostProcess::sobelEdgeDetection(zbuffer, stencilbuffer, normalbuffer, width, height, image, outerlinecolor, innerlinecolor, true, stencilbuffervalue);
    PostProcess::Flare(width, height, image);
    std::cout << "Finished PostProcess." << std::endl;
}

TGAColor ToonRenderPipeline::fragmentShader(int x, int y)
{
    if (blockidx == 6) {
        // 如果是面部则利用中心点重新计算法线
        clampnormal = normalize(clampWP - facecenter);
    }

    //// 计算光照强度
    float ndotl = dot(clampnormal, normalize(lightdir));
    float ndotv = dot(clampnormal, normalize(cameraPos - clampWP));
    //ndotl = fmax(0, ndotl);  // 为实现面部的正确阴影ndotl需要在-1.0到1.0之间
    ndotv = fmax(0, ndotv);
    // 二维Ramp采样
    vec4f rampcolor = ramptex.samplerTexure(vec2f(ndotl, ndotv));
    //vec4f rampcolor(1.0, 1.0, 1., 1.);

    // 边缘光
    // float fresnelvalue = 1. + pow(1. - ndotv, 7) / 2.; // ndotv效果不好
    // 视角空间法线偏移的等宽边缘光

    float offsetdepthVS;
    float depthVS;
    switch (aa)
    {
    case MSAA:
        offsetdepthVS = 0;
        depthVS = 0;
        break;
    default:
        offsetdepthVS = depthbufferVS[static_cast<int>(clampoffset.x) * height + static_cast<int>(clampoffset.y)];
        depthVS = depthbufferVS[x * height + y];
        break;
    }

    float diffdepthVS = abs(depthVS - offsetdepthVS);
    diffdepthVS = fmax(diffdepthVS - 10., 0.0);
    if (blockidx == 6) {
        diffdepthVS = fmax(diffdepthVS - 10., 0.0);
    }
    vec4c color = objfiles.samplerTexture2D(objfiles.fromBlockIdx2TextureIdx(blockidx), clampUV);

    vec4f tempcolor = rampcolor * (1 + fmin(1.0, diffdepthVS));
    TGAColor rescolor = TGAColor(static_cast<unsigned char>(fmin(static_cast<float>(color.x) * tempcolor.x, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.y) * tempcolor.y, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.z) * tempcolor.z, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.w) * tempcolor.w, 255)));

    return rescolor;
}

void ToonRenderPipeline::writeTexture(int _x, int _y, TGAColor rescolor)
{
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

void ToonRenderPipeline::Draw()
{
    if (aa == AntiAliasing::SSAA) {
        DownsampleSSAA();
        ssaa_downsampler_img.flip_vertically();
        ssaa_downsampler_img.write_tga_file("Resource/SSAA.tga");
    }
    else if (aa == AntiAliasing::MSAA) {
        int originalwidth = width / msaa_param;
        int originalheight = height / msaa_param;

        for (int y = 0; y < originalheight; ++y) {
            for (int x = 0; x < originalwidth; ++x) {
                int sumR = 0, sumG = 0, sumB = 0, sumA = 0;

                for (int isampleX = 0; isampleX < msaa_param; ++isampleX) {
                    for (int isampleY = 0; isampleY < msaa_param; ++isampleY) {
                        int sampleindexX = x * msaa_param + isampleX;
                        int sampleindexY = y * msaa_param + isampleY;
                        TGAColor c = image.get(sampleindexX, sampleindexY);
                        sumR += c.r;
                        sumG += c.g;
                        sumB += c.b;
                        sumA += c.a;
                    }
                }

                int total = msaa_param * msaa_param;
                unsigned char avgR = static_cast<unsigned char>(sumR / total);
                unsigned char avgG = static_cast<unsigned char>(sumG / total);
                unsigned char avgB = static_cast<unsigned char>(sumB / total);
                unsigned char avgA = static_cast<unsigned char>(sumA / total);
                TGAColor avgColor(avgR, avgG, avgB, avgA);
                colorMSAABuffer.set(x, y, avgColor);
            }
        }
        colorMSAABuffer.flip_vertically();
        colorMSAABuffer.write_tga_file("Resource/MSAA.tga");
        image.flip_vertically();
        image.write_tga_file("Resource/output.tga");
    }
    else {
        image.flip_vertically();
        image.write_tga_file("Resource/output.tga");
        normalbuffer.flip_vertically();
        normalbuffer.write_tga_file("Resource/NormalBuffer.tga");
    }
}

void ToonRenderPipeline::customDataSet()
{
    // 获取深度值
    drender.Commit();
    depthbufferVS = drender.getZBuffer();

    // 设置shader使用的变量
    facecenter = objfiles.getVert(objfiles.getFace(7, 0)[0].x);
    facecenter = modelmat.MultipleVec4(vec4f(facecenter, 1.0)).xyz();
    ramptex = Texture("Resource/Ramp2D.png");
}

void ToonRenderPipeline::Pipeline()
{
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
}

void ToonRenderPipeline::rasterize()
{
    switch (aa)
    {
    case NoAA:
        NoAARasterize();
        break;
    case SSAA:
        NoAARasterize(); // 这里计算的是扩大分辨率的颜色缓冲
        break;
    case MSAA:
        MSAARasterize();
        break;
    default:
        break;
    }
}

void ToonRenderPipeline::setObjFile(OBJParser& f)
{
    objfiles = f;
    drender.setObjFile(objfiles);
}

void ToonRenderPipeline::setModelTransform(Matrix rotation, Matrix location)
{
    modelmat = location.MultipleMat(rotation);
    drender.setModelTransform(rotation, location);
}

void ToonRenderPipeline::setCamera(vec3f cameraPos, vec3f cameraRot, int f, int n, float fov, float aspect) 
{
    RenderPipeline::setCamera(cameraPos, cameraRot, f, n, fov, aspect);
    drender.setCamera(cameraPos, cameraRot, 10, 0.1, 45, 1);
}

void ToonRenderPipeline::NoAARasterize()
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

void ToonRenderPipeline::MSAARasterize()
{
    // 检查是否需要丢弃图元
    bool isthrow = true;
    for (int i = 0; i < 3; ++i) {
        if (screen_coords[i].x < width / msaa_param && screen_coords[i].y < height / msaa_param
            && screen_coords[i].x > 0 && screen_coords[i].y > 0) {
            isthrow = false;
            break;
        }
    }
    if (isthrow) return;

    // 计算包围盒
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

    for (int _x = (int)round(vertmin) + 0.5; _x <= vertmax; ++_x) {
        for (int _y = (int)round(horimin) + 0.5; _y <= horimax; ++_y) {
            // 计算重心坐标
            vec3f _cv = calculateBarycentricCoordinates(screen_coords, _x, _y);
            // 判断是否在三角形内部
            if (_cv.x >= 0 && _cv.y >= 0 && _cv.x + _cv.y <= 1) {
                // 丢弃画面外的像素点
                if (_x >= width || _y >= height || _x < 0 || _y < 0) {
                    continue;
                }
                // 插值计算深度
                clampZ(_cv);
                // 自定义插值
                clampInTriangle(_cv);
                // 计算片元颜色
                TGAColor rescolor = fragmentShader(_x + 0.5, _y + 0.5);

                for (int isampleX = 0; isampleX < msaa_param; ++isampleX) {
                    for (int isampleY = 0; isampleY < msaa_param; ++isampleY) {
                        // 计算当前采样点的屏幕坐标
                        float sampleX = _x + static_cast<float>(isampleX) / msaa_param;
                        float sampleY = _y + static_cast<float>(isampleY) / msaa_param;

                        // 计算重心坐标
                        vec3f _cv_t = calculateBarycentricCoordinates(screen_coords, sampleX, sampleY);
                        float _z_t = (1 - _cv_t.x - _cv_t.y) * screen_coords[0].z + _cv_t.x * screen_coords[1].z + _cv_t.y * screen_coords[2].z;

                        // 检查是否在三角形内部
                        if (_cv_t.x >= 0 && _cv_t.y >= 0 && _cv_t.x + _cv_t.y <= 1) {
                            int sampleindexX = _x * msaa_param + isampleX;
                            int sampleindexY = _y * msaa_param + isampleY;
                            int sample_buffer_index = (_x * height / msaa_param + _y) * (msaa_param * msaa_param) + isampleX * msaa_param + isampleY;

                            // 深度和模板测试
                            if (zbuffer[sample_buffer_index] > _z_t) {
                                zbuffer[sample_buffer_index] = _z_t;
                                image.set(sampleindexX, sampleindexY, rescolor);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ToonRenderPipeline::geometryVertexShader(int blockidx, int faceidx)
{
    face_info = objfiles.getFace(blockidx, faceidx);
    this->blockidx = blockidx;

    world_coords.clear();
    screen_coords.clear();
    vertex_normals.clear();
    vertex_uv.clear();
    offset_vertexCS.clear();

    for (int j = 0; j < 3; j++) {
        // 模型变换后不需要进行齐次除法
        world_coords.push_back(modelmat.MultipleVec4(vec4f(objfiles.getVert(face_info[j].x), 1.)));
        vec4f VSvertex = viewmat.MultipleVec4(world_coords[j]);
        vec4f cilp_space_coord = projectionMat.MultipleVec4(VSvertex);

        // 齐次除法
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;

        switch (aa)
        {
        case AntiAliasing::MSAA:
            screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width / msaa_param - 1) / 2., (cilp_space_coord.y + 1.) * (height / msaa_param - 1) / 2., -cilp_space_coord.z));
            break;
        default:
            screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));
            break;
        }

        // 添加对应的顶点法线
        vertex_normals.push_back(objfiles.getNormals(face_info[j].z));

        // 添加对应的uv坐标
        vertex_uv.push_back(objfiles.getUV(face_info[j].y));

        // 视角空间下偏移顶点，转换到屏幕坐标下
        vec3f VSnormal = viewmat.MultipleNormal(vertex_normals[j]);
        vec4f VSoffsetvertex(VSvertex.xyz() + VSnormal * 0.02 * rimlightwidth, 1.0);

        cilp_space_coord = projectionMat.MultipleVec4(VSoffsetvertex);
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;
        switch (aa)
        {
        case AntiAliasing::MSAA:
            offset_vertexCS.push_back(vec3f((cilp_space_coord.x + 1.) * (width / msaa_param  - 1) / 2., (cilp_space_coord.y + 1.) * (height / msaa_param - 1) / 2., -cilp_space_coord.z));
            break;
        default:
            offset_vertexCS.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));
            break;
        }
    }
}

void ToonRenderPipeline::drawHairCastShadow(int bidx)
{
    for (int faceidx = 0; faceidx < objfiles.nFaces(bidx); ++faceidx) {
        face_info = objfiles.getFace(bidx, faceidx);
        this->blockidx = bidx;

        world_coords.clear();
        screen_coords.clear();

        float offset = 0.02;
        for (int j = 0; j < 3; j++) {
            world_coords.push_back(modelmat.MultipleVec4(vec4f(objfiles.getVert(face_info[j].x), 1.)));
            world_coords[j] = vec4f(world_coords[j].x - lightdir.x * offset,
                world_coords[j].y - lightdir.y * offset,
                world_coords[j].z - lightdir.z * offset,
                1.0);
            vec4f cilp_space_coord = projectionMat.MultipleVec4(viewmat.MultipleVec4(world_coords[j]));

            // 齐次除法
            cilp_space_coord.x /= cilp_space_coord.w;
            cilp_space_coord.y /= cilp_space_coord.w;

            screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));
        }

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
                    // 片元着色器
                    vec4f shadowcolor(0.97, 0.94, 0.93, 1.);

                    // 深度模板测试
                    if (zbuffer[_x * height + _y] > clampz && stencilbuffer[_x * height + _y] == stencilbuffervalue) {
                        TGAColor ori_color = image.get(_x, height - _y);
                        unsigned char t = static_cast<unsigned char>(fmin(static_cast<float>(ori_color.r) * shadowcolor.x, 255));
                        //decreaseLS(ori_color.r, ori_color.r, ori_color.r)
                        image.set(_x, height - _y, TGAColor(static_cast<unsigned char>(fmin(static_cast<float>(ori_color.r) * shadowcolor.x, 255)),
                            static_cast<unsigned char>(fmin(static_cast<float>(ori_color.g) * shadowcolor.y, 255)),
                            static_cast<unsigned char>(fmin(static_cast<float>(ori_color.b) * shadowcolor.z, 255)),
                            static_cast<unsigned char>(fmin(static_cast<float>(ori_color.a) * shadowcolor.w, 255))));
                    }
                }
            }
        }
    }
    std::cout << "Finished render block number alone: " << bidx << std::endl;
    image.write_tga_file("Resource/output.tga");
}

void ToonRenderPipeline::DownsampleSSAA() {
    // 计算原始图像的尺寸
    int ssaa_width = image.get_width();
    int ssaa_height = image.get_height();
    int orig_width = ssaa_width / ssaa_param;
    int orig_height = ssaa_height / ssaa_param;
    int bytespp = image.get_bytespp();

    // 遍历原始图像的每个像素
    for (int y = 0; y < orig_height; y++) {
        for (int x = 0; x < orig_width; x++) {
            // 使用整型来避免溢出
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            // 对应 SSAA 图像中的 n×n 块
            for (int j = 0; j < ssaa_param; j++) {
                for (int i = 0; i < ssaa_param; i++) {
                    TGAColor c = image.get(x * ssaa_param + i, y * ssaa_param + j);
                    sumR += c.r;
                    sumG += c.g;
                    sumB += c.b;
                    sumA += c.a;
                }
            }
            int total = ssaa_param * ssaa_param;
            unsigned char avgR = static_cast<unsigned char>(sumR / total);
            unsigned char avgG = static_cast<unsigned char>(sumG / total);
            unsigned char avgB = static_cast<unsigned char>(sumB / total);
            unsigned char avgA = static_cast<unsigned char>(sumA / total);
            TGAColor avgColor(avgR, avgG, avgB, avgA);
            ssaa_downsampler_img.set(x, y, avgColor);
        }
    }
    return;
}
