#include "math.h"
#include <cmath>
#include <cstdio>

Mat4 Mat4_Create() {
    Mat4 m4 = {
        .data = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
        }
    };

    return m4;
}

Mat4 Mat4_Translate(Mat4 mat4, Vec3 vec3) {
    float *m = mat4.data;
    float tx = vec3.x;
    float ty = vec3.y;
    float tz = vec3.z;

    Mat4 result = {
        .data = {
            m[0] + m[12] * tx, m[1] + m[13] * tx, m[2] + m[14] * tx, m[3] + m[15] * tx,
            m[4] + m[12] * ty, m[5] + m[13] * ty, m[6] + m[14] * ty, m[7] + m[15] * ty,
            m[8] + m[12] * tz, m[9] + m[13] * tz, m[10] + m[14] * tz, m[11] + m[15] * tz,
            m[12], m[13], m[14], m[15],
        }
    };

    return result;
}

Mat4 Mat4_Scale(Mat4 mat4, Vec3 vec3) {
    float *m = mat4.data;
    float sx = vec3.x;
    float sy = vec3.y;
    float sz = vec3.z;

    Mat4 result = {
        .data = {
            m[0] * sx, m[1] * sx, m[2] * sx,  m[3] * sx,
            m[4] * sy, m[5] * sy, m[6] * sy,  m[7] * sy,
            m[8] * sz, m[9] * sz, m[10] * sz, m[11] * sz,
            m[12],     m[13],     m[14],      m[15],
        }
    };

    return result;
}

// Perspective projection matrix (OpenGL convention)
// | 1/(aspect·tan(fov/2))       0              0              0  |
// |          0            1/tan(fov/2)         0              0  |
// |          0                  0         -(f+n)/(f-n)   -2fn/(f-n)|
// |          0                  0             -1              0  |
Mat4 Mat4_Perspective(float fovy, float aspect, float zNear, float zFar) {
    Mat4 result = Mat4_Create();

    float t = tan(fovy / 2);

    result.data[0] = 1 / (aspect * t);
    result.data[5] = 1 / t;
    result.data[10] = -(zFar+zNear)/(zFar-zNear);
    result.data[11] = -2*zFar*zNear/(zFar-zNear);
    result.data[14] = -1;
    result.data[15] = 0.0f;

    return result;
}


// Orthographic projection matrix (OpenGL convention)
// | 2/(r-l)     0        0      -(r+l)/(r-l) |
// |    0     2/(t-b)     0      -(t+b)/(t-b) |
// |    0        0    -2/(f-n)   -(f+n)/(f-n) |
// |    0        0        0            1      |
Mat4 Mat4_Ortho(float left, float right, float bottom, float top, float zNear,
                float zFar) {
    Mat4 result = Mat4_Create();

    result.data[0] = 2 / (right - left);
    result.data[3] = -(right + left) / (right - left);
    result.data[5] = 2 / (top - bottom);
    result.data[7] = -(top + bottom) / (top - bottom);
    result.data[10] = -2 / (zFar - zNear);
    result.data[11] = -(zFar + zNear) / (zFar - zNear);
    result.data[15] = 1.0f;

    return result;
}

Vec4 Vec4_Transform(Vec4 vec4, Mat4 mat4) {
    float *m = mat4.data;

    Vec4 result = {
        vec4.x * m[0] + vec4.y * m[1] + vec4.z * m[2] + vec4.w * m[3],
        vec4.x * m[4] + vec4.y * m[5] + vec4.z * m[6] + vec4.w * m[7],
        vec4.x * m[8] + vec4.y * m[9] + vec4.z * m[10] + vec4.w * m[11],
        vec4.x * m[12] + vec4.y * m[13] + vec4.z * m[14] + vec4.w * m[15],
    };

    return result;
}

float DegToRadians(float deg) {
    return deg * PI / 180.0f;
}
