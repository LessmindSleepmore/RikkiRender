#include "../../Header/Render/ToonRender.h"

void ToonRender::clampInTriangle(vec3f _cv)
{
    clampnormal = normalize(vertex_normals[0] * (1 - _cv.x - _cv.y) + vertex_normals[1] * _cv.x + vertex_normals[2] * _cv.y);
    clampWP = world_coords[0].xyz() * (1 - _cv.x - _cv.y) + world_coords[1].xyz() * _cv.x + world_coords[2].xyz() * _cv.y;
    clampUV = vertex_uv[0] * (1 - _cv.x - _cv.y) + vertex_uv[1] * _cv.x + vertex_uv[2] * _cv.y;
    clampoffset = offset_vertexCS[0] * (1 - _cv.x - _cv.y) + offset_vertexCS[1] * _cv.x + offset_vertexCS[2] * _cv.y;
}

void ToonRender::postProcess()
{
    // ����ߺ��������ñ�Ե���ʵ�֣�
    PostProcess::sobelEdgeDetection(zbuffer, stencilbuffer, normalbuffer, width, height, image, outerlinecolor, innerlinecolor, true, stencilbuffervalue);
    PostProcess::Flare(width, height, image);
    std::cout << "Finished PostProcess." << std::endl;
}

TGAColor ToonRender::fragmentShader()
{
    if (blockidx == 6) {
        // ������沿���������ĵ����¼��㷨��
        clampnormal = normalize(clampWP - facecenter);
    }

    //// �������ǿ��
    float ndotl = dot(clampnormal, normalize(lightdir));
    float ndotv = dot(clampnormal, normalize(cameraPos - clampWP));
    //ndotl = fmax(0, ndotl);  // Ϊʵ���沿����ȷ��Ӱndotl��Ҫ��-1.0��1.0֮��
    ndotv = fmax(0, ndotv);
    // ��άRamp����
    vec4f rampcolor = ramptex.samplerTexure(vec2f(ndotl, ndotv));
    //vec4f rampcolor(1.0, 1.0, 1., 1.);

    // ��Ե��
    // float fresnelvalue = 1. + pow(1. - ndotv, 7) / 2.; // ndotvЧ������
    // �ӽǿռ䷨��ƫ�Ƶĵȿ��Ե��
    float offsetdepthVS = depthbufferVS[static_cast<int>(clampoffset.x) * height + static_cast<int>(clampoffset.y)];
    float depthVS = depthbufferVS[_x * height + _y];

    float diffdepthVS = abs(depthVS - offsetdepthVS);
    diffdepthVS = fmax(diffdepthVS - 10., 0.0);

    vec4c color = objfiles.samplerTexture2D(objfiles.fromBlockIdx2TextureIdx(blockidx), clampUV);
    vec4f tempcolor = rampcolor *  (1 + fmin(1.0, diffdepthVS));
    TGAColor rescolor = TGAColor(static_cast<unsigned char>(fmin(static_cast<float>(color.x) * tempcolor.x, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.y) * tempcolor.y, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.z) * tempcolor.z, 255)),
        static_cast<unsigned char>(fmin(static_cast<float>(color.w) * tempcolor.w, 255)));

    return rescolor;
}

void ToonRender::writeTexture(int _x, int _y, TGAColor rescolor)
{
    image.set(_x, _y, rescolor);
    normalbuffer.set(_x, _y, TGAColor(abs(clampnormal.x) * 255, abs(clampnormal.y) * 255, abs(clampnormal.z) * 255, 255));

    if (enablestencilwrite) {
        stencilbuffer[_x * height + _y] = stencilbuffervalue;
    }
    else {
        // ģ�建������Ϊ��ʼֵ
        stencilbuffer[_x * height + _y] = 0;
    }
}

void ToonRender::Draw()
{
    image.flip_vertically();
    image.write_tga_file("Resource/output.tga");
    normalbuffer.flip_vertically();
    normalbuffer.write_tga_file("Resource/NormalBuffer.tga");
}

void ToonRender::customDataSet()
{
    // ����shaderʹ�õı���
    facecenter = objfiles.getVert(objfiles.getFace(7, 0)[0].x);
    facecenter = modelmat.MultipleVec4(vec4f(facecenter, 1.0)).xyz();
    ramptex = Texture("Resource/Ramp2D.png");
}

void ToonRender::Pipeline()
{
    for (int blockidx = 0; blockidx < objfiles.nBlock(); ++blockidx) {

        // �����沿д��ģ��ֵ�������к������
        if (blockidx == 6 || blockidx == 4) stencilTestSettting(true, 1);
        else stencilTestSettting(false, 1);

        for (int faceidx = 0; faceidx < objfiles.nFaces(blockidx); ++faceidx) {
            geometryVertexShader(blockidx, faceidx);

            rasterize();
        }
        std::cout << "Finished render block number: " << blockidx << std::endl;
    }
}

void ToonRender::geometryVertexShader(int blockidx, int faceidx)
{
    face_info = objfiles.getFace(blockidx, faceidx);
    this->blockidx = blockidx;

    world_coords.clear();
    screen_coords.clear();
    vertex_normals.clear();
    vertex_uv.clear();
    offset_vertexCS.clear();

    for (int j = 0; j < 3; j++) {
        // ģ�ͱ任����Ҫ������γ���
        world_coords.push_back(modelmat.MultipleVec4(vec4f(objfiles.getVert(face_info[j].x), 1.)));
        vec4f VSvertex = viewmat.MultipleVec4(world_coords[j]);
        vec4f cilp_space_coord = projectionMat.MultipleVec4(VSvertex);

        // ��γ���
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;

        screen_coords.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));

        // ��Ӷ�Ӧ�Ķ��㷨��(����û�任���������꣬��Ϊû�ƶ���תģ������û����)
        vertex_normals.push_back(objfiles.getNormals(face_info[j].z));

        // ��Ӷ�Ӧ��uv����
        vertex_uv.push_back(objfiles.getUV(face_info[j].y));

        // �ӽǿռ���ƫ�ƶ��㣬ת������Ļ������
        vec3f VSnormal = viewmat.MultipleNormal(vertex_normals[j]);
        vec4f VSoffsetvertex(VSvertex.xyz() + VSnormal * 0.02 * rimlightwidth, 1.0);

        cilp_space_coord = projectionMat.MultipleVec4(VSoffsetvertex);
        cilp_space_coord.x /= cilp_space_coord.w;
        cilp_space_coord.y /= cilp_space_coord.w;
        offset_vertexCS.push_back(vec3f((cilp_space_coord.x + 1.) * (width - 1) / 2., (cilp_space_coord.y + 1.) * (height - 1) / 2., -cilp_space_coord.z));
    }
}
