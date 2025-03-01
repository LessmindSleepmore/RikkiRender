#include "GeometricTypes.h"

#define M_PI 3.14159265358979323846

Matrix::Matrix()
{
	raw = std::vector<std::vector<float>>(4, std::vector<float>(4, 0));
}

Matrix::Matrix(int dim)
{
	raw = std::vector<std::vector<float>>(dim, std::vector<float>(dim, 0));
}

Matrix::Matrix(int dim, float defaultValue)
{
	raw = std::vector<std::vector<float>>(dim, std::vector<float>(dim, defaultValue));
}

/**
* 右手坐标系旋转矩阵，顺序X，Y，Z
*/
Matrix::Matrix(float pitch, float yaw, float roll)
{
    raw = std::vector<std::vector<float>>(4, std::vector<float>(4, 0));

    float cosPitch = cos(d2r(pitch));
    float sinPitch = sin(d2r(pitch));
    float cosYaw = cos(d2r(yaw));
    float sinYaw = sin(d2r(yaw));
    float cosRoll = cos(d2r(roll));
    float sinRoll = sin(d2r(roll));

    // 填充旋转矩阵元素
    raw[0][0] = cosRoll * cosYaw;
    raw[0][1] = cosRoll * sinYaw * sinPitch - sinRoll * cosPitch;
    raw[0][2] = cosRoll * sinYaw * cosPitch + sinRoll * sinPitch;
    raw[0][3] = 0;

    raw[1][0] = sinRoll * cosYaw;
    raw[1][1] = sinRoll * sinYaw * sinPitch + cosRoll * cosPitch;
    raw[1][2] = sinRoll * sinYaw * cosPitch - cosRoll * sinPitch;
    raw[1][3] = 0;

    raw[2][0] = -sinYaw;
    raw[2][1] = cosYaw * sinPitch;
    raw[2][2] = cosYaw * cosPitch;
    raw[2][3] = 0;

    raw[3][0] = 0;
    raw[3][1] = 0;
    raw[3][2] = 0;
    raw[3][3] = 1;
}

Matrix::Matrix(vec3f target)
{
    raw = std::vector<std::vector<float>>(4, std::vector<float>(4, 0));
    
    raw[0][0] = 1;
    raw[1][1] = 1;
    raw[2][2] = 1;
    raw[3][3] = 1;
    raw[0][3] = target.x;
    raw[1][3] = target.y;
    raw[2][3] = target.z;
}

Matrix::Matrix(vec3f camPos, vec3f camRot)
{
    Matrix rotMat(camRot.x, camRot.y, camRot.z);
    Matrix transMat(vec3f(-camPos.x, -camPos.y, -camPos.z));
    Matrix inversMat = rotMat.inverse();

    // 先平移再旋转
    raw = rotMat.MultipleMat(transMat).GetRaw();
}

Matrix::~Matrix()
{
}

void Matrix::SetValue(int x, int y, float value)
{
	raw[x][y] = value;
}

float Matrix::GetValue(int x, int y)
{
    return raw[x][y];
}

std::vector<std::vector<float>> Matrix::GetRaw()
{
    return raw;
}

Matrix Matrix::MakeProjectionMatrix(int f, int n, float fov, float aspect)
{
    Matrix res(4, 0);

    float fovRad = 1.0 / tan(fov / 180. * M_PI / 2.);

    res.SetValue(0, 0, fovRad / aspect / 2);
    res.SetValue(1, 1, fovRad / 2);
    res.SetValue(2, 2, (n + f) / static_cast<float>(n - f));
    res.SetValue(2, 3, -2 * f * n / static_cast<float>(n - f));
    res.SetValue(3, 2, 1);
    return res;
}

vec4f Matrix::MultipleVec4(vec4f v)
{
    std::vector<float> result4(4, 0);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result4[i] += raw[i][j] * v.raw[j];
        }
    }

    vec4f result;
    result.x = result4[0];
    result.y = result4[1];
    result.z = result4[2];
    result.w = result4[3];

    return result;
}

vec3f Matrix::MultipleVec3(vec3f v) {
    // 将三维向量扩展为四维齐次坐标向量 (x, y, z, 1)
    std::vector<float> vec4 = { v.x, v.y, v.z, 1.0f };
    std::vector<float> result4(4, 0);

    // 矩阵乘法
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result4[i] += raw[i][j] * vec4[j];
        }
    }

    // 将结果转换回三维向量
    vec3f result;
    result.x = result4[0];
    result.y = result4[1];
    result.z = result4[2];

    return result;
}

Matrix Matrix::MultipleMat(Matrix m)
{
    Matrix res(4, 0);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0;
            for (int k = 0; k < 4; ++k) {
                sum += this->GetValue(i, k) * m.GetValue(k, j);
            }
            res.SetValue(i, j, sum);
        }
    }
    return res;
}

Matrix Matrix::inverse()
{
    Matrix inv(4, 0);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            inv.raw[i][j] = raw[j][i];
        }
    }
    return inv;
}

double Matrix::d2r(float degrees)
{
	return degrees / 180. * M_PI;
}
