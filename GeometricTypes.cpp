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

Matrix::Matrix(float pitch, float yaw, float roll)
{
    raw = std::vector<std::vector<float>>(4, std::vector<float>(4, 0));
    float cosPitch = cos(d2r(pitch));
    float sinPitch = sin(d2r(pitch));
    float cosYaw = cos(d2r(yaw));
    float sinYaw = sin(d2r(yaw));
    float cosRoll = cos(d2r(roll));
    float sinRoll = sin(d2r(roll));

    // Ìî³äÐý×ª¾ØÕóÔªËØ
    raw[0][0] = cosYaw * cosRoll;
    raw[0][1] = cosYaw * sinRoll * sinPitch - sinYaw * cosPitch;
    raw[0][2] = cosYaw * sinRoll * cosPitch + sinYaw * sinPitch;
    raw[0][3] = 0;

    raw[1][0] = sinYaw * cosRoll;
    raw[1][1] = sinYaw * sinRoll * sinPitch + cosYaw * cosPitch;
    raw[1][2] = sinYaw * sinRoll * cosPitch - cosYaw * sinPitch;
    raw[1][3] = 0;

    raw[2][0] = -sinRoll;
    raw[2][1] = cosRoll * sinPitch;
    raw[2][2] = cosRoll * cosPitch;
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

vec3f Matrix::MultipleVec3(vec3f v)
{
    std::vector<float> vec4 = { v.x, v.y, v.z, 1.0f };
    std::vector<float> result4(4, 0);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result4[i] += raw[i][j] * vec4[j];
        }
    }

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

double Matrix::d2r(float degrees)
{
	return degrees / 180. * M_PI;
}
