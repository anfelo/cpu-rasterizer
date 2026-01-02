#include "math.h"
#include <algorithm>
#include <cmath>

Mat4 Mat4_Create() {
    Mat4 m4 = {.data = {
                   1.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   1.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   1.0,
                   0.0,
                   0.0,
                   0.0,
                   0.0,
                   1.0,
               }};

    return m4;
}

Mat4 Mat4_Mult(Mat4 matA, Mat4 matB) {
    float *a = matA.data;
    float *b = matB.data;

    Mat4 result = {
        .data = {
            a[0] * b[0] + a[1] * b[4] + a[2] * b[8] + a[3] * b[12],
            a[0] * b[1] + a[1] * b[5] + a[2] * b[9] + a[3] * b[13],
            a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14],
            a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15],

            a[4] * b[0] + a[5] * b[4] + a[6] * b[8] + a[7] * b[12],
            a[4] * b[1] + a[5] * b[5] + a[6] * b[9] + a[7] * b[13],
            a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14],
            a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15],

            a[8] * b[0] + a[9] * b[4] + a[10] * b[8] + a[11] * b[12],
            a[8] * b[1] + a[9] * b[5] + a[10] * b[9] + a[11] * b[13],
            a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14],
            a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15],

            a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + a[15] * b[12],
            a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + a[15] * b[13],
            a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14],
            a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15],
        }};

    return result;
}

Mat4 Mat4_Translate(Mat4 mat4, Vec3 vec3) {
    float tx = vec3.x;
    float ty = vec3.y;
    float tz = vec3.z;

    Mat4 T = Mat4_Create();
    T.data[3] = tx;
    T.data[7] = ty;
    T.data[11] = tz;

    Mat4 result = Mat4_Mult(T, mat4);

    return result;
}

Mat4 Mat4_Scale(Mat4 mat4, Vec3 vec3) {
    float sx = vec3.x;
    float sy = vec3.y;
    float sz = vec3.z;

    Mat4 S = Mat4_Create();
    S.data[0] = sx;
    S.data[5] = sy;
    S.data[10] = sz;

    Mat4 result = Mat4_Mult(S, mat4);

    return result;
}

Mat4 Mat4_RotateX(Mat4 mat4, float deg) {
    float *m = mat4.data;
    float c = cos(DegToRadians(deg));
    float s = sin(DegToRadians(deg));

    Mat4 result = {.data = {
                       m[0],
                       m[1],
                       m[2],
                       m[3],
                       m[4] * c - m[8] * s,
                       m[5] * c - m[9] * s,
                       m[6] * c - m[10] * s,
                       m[7] * c - m[11] * s,
                       m[4] * s + m[8] * c,
                       m[5] * s + m[9] * c,
                       m[6] * s + m[10] * c,
                       m[7] * s + m[11] * c,
                       m[12],
                       m[13],
                       m[14],
                       m[15],
                   }};

    return result;
}

Mat4 Mat4_RotateY(Mat4 mat4, float deg) {
    float *m = mat4.data;
    float c = cos(DegToRadians(deg));
    float s = sin(DegToRadians(deg));

    Mat4 result = {.data = {
                       m[0] * c + m[8] * s,
                       m[1] * c + m[9] * s,
                       m[2] * c + m[10] * s,
                       m[3] * c + m[11] * s,
                       m[4],
                       m[5],
                       m[6],
                       m[7],
                       -m[0] * s + m[8] * c,
                       -m[1] * s + m[9] * c,
                       -m[2] * s + m[10] * c,
                       -m[3] * s + m[11] * c,
                       m[12],
                       m[13],
                       m[14],
                       m[15],
                   }};

    return result;
}

Mat4 Mat4_RotateZ(Mat4 mat4, float deg) {
    float *m = mat4.data;
    float c = cos(DegToRadians(deg));
    float s = sin(DegToRadians(deg));

    Mat4 result = {.data = {
                       m[0] * c - m[4] * s,
                       m[1] * c - m[5] * s,
                       m[2] * c - m[6] * s,
                       m[3] * c - m[7] * s,
                       m[0] * s + m[4] * c,
                       m[1] * s + m[5] * c,
                       m[2] * s + m[6] * c,
                       m[3] * s + m[7] * c,
                       m[8],
                       m[9],
                       m[10],
                       m[11],
                       m[12],
                       m[13],
                       m[14],
                       m[15],
                   }};

    return result;
}

Mat4 Mat4_Rotate(Mat4 mat4, Vec3 vec3) {
    Mat4 result;
    result = Mat4_RotateX(mat4, vec3.x);
    result = Mat4_RotateY(result, vec3.y);
    result = Mat4_RotateZ(result, vec3.z);

    return result;
}

// Perspective projection matrix (OpenGL convention)
// | 1/(aspect·tan(fov/2))       0              0              0  |
// |          0            1/tan(fov/2)         0              0  |
// |          0                  0         -(f+n)/(f-n)   -2fn/(f-n)|
// |          0                  0             -1              0  |
Mat4 Mat4_Perspective(float fovy, float aspect, float zNear, float zFar) {
    Mat4 result = Mat4_Create();

    float t = tanf(fovy / 2.0f);

    result.data[0] = 1.0f / (aspect * t);
    result.data[5] = 1.0f / t;
    result.data[10] = -(zFar + zNear) / (zFar - zNear);
    result.data[11] = -1.0f;
    result.data[14] = -2.0f * zFar * zNear / (zFar - zNear);
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

Mat4 Mat4_LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 f = Vec3_Subtract(target, eye); // forward

    // Guard against zero-length forward vector
    if (Vec3_Mag(f) < 0.0001f) {
        return Mat4_Create();
    }
    f = Vec3_Normalize(f);

    Vec3 r = Vec3_Normalize(Vec3_Cross(f, up)); // right
    Vec3 u = Vec3_Cross(r, f);                  // true up

    Mat4 out;

    // Column-major order (OpenGL convention)
    out.data[0] = r.x;
    out.data[1] = u.x;
    out.data[2] = -f.x;
    out.data[3] = 0.0f;

    out.data[4] = r.y;
    out.data[5] = u.y;
    out.data[6] = -f.y;
    out.data[7] = 0.0f;

    out.data[8] = r.z;
    out.data[9] = u.z;
    out.data[10] = -f.z;
    out.data[11] = 0.0f;

    out.data[12] = -Vec3_Dot(r, eye);
    out.data[13] = -Vec3_Dot(u, eye);
    out.data[14] = Vec3_Dot(f, eye);
    out.data[15] = 1.0f;

    return out;
}

Mat4 Mat4_Transpose(Mat4 mat4) {
    float *m = mat4.data;

    Mat4 result;

    result.data[0] = m[0];
    result.data[1] = m[4];
    result.data[2] = m[8];
    result.data[3] = m[12];

    result.data[4] = m[1];
    result.data[5] = m[5];
    result.data[6] = m[9];
    result.data[7] = m[13];

    result.data[8] = m[2];
    result.data[9] = m[6];
    result.data[10] = m[10];
    result.data[11] = m[14];

    result.data[12] = m[3];
    result.data[13] = m[7];
    result.data[14] = m[11];
    result.data[15] = m[15];

    return result;
}

float Vec3_Mag(Vec3 vec3) {
    return sqrtf(vec3.x * vec3.x + vec3.y * vec3.y + vec3.z * vec3.z);
}

Vec3 Vec3_ScalarMult(Vec3 vec3, float value) {
    return {vec3.x * value, vec3.y * value, vec3.z * value};
}

Vec3 Vec3_ScalarDivide(Vec3 vec3, float value) {
    return {vec3.x / value, vec3.y / value, vec3.z / value};
}

Vec3 Vec3_Mult(Vec3 a, Vec3 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

Vec3 Vec3_Subtract(Vec3 a, Vec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 Vec3_Add(Vec3 a, Vec3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 Vec3_Normalize(Vec3 vec3) {
    float mag = Vec3_Mag(vec3);

    if (mag == 0.0f) {
        return vec3;
    }

    return Vec3_ScalarDivide(vec3, mag);
}

float Vec3_Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

Vec3 Vec3_Reflect(Vec3 incident, Vec3 normal) {
    float d = Vec3_Dot(normal, incident);
    return (Vec3){incident.x - 2.0f * d * normal.x,
                  incident.y - 2.0f * d * normal.y,
                  incident.z - 2.0f * d * normal.z};
}

Vec4 Vec4_Transform(Vec4 vec4, Mat4 mat4) {
    float *m = mat4.data;

    Vec4 result = {
        vec4.x * m[0] + vec4.y * m[4] + vec4.z * m[8] + vec4.w * m[12],
        vec4.x * m[1] + vec4.y * m[5] + vec4.z * m[9] + vec4.w * m[13],
        vec4.x * m[2] + vec4.y * m[6] + vec4.z * m[10] + vec4.w * m[14],
        vec4.x * m[3] + vec4.y * m[7] + vec4.z * m[11] + vec4.w * m[15],
    };

    return result;
}

float DegToRadians(float deg) {
    return deg * PI / 180.0f;
}

uint32_t ColorToInt(ColorRGBA color) {
    uint8_t red = static_cast<uint8_t>(std::min(color.r * 255.0f, 255.0f));
    uint8_t green = static_cast<uint8_t>(std::min(color.g * 255.0f, 255.0f));
    uint8_t blue = static_cast<uint8_t>(std::min(color.b * 255.0f, 255.0f));

    return (0xFF << 24) | (red << 16) | (green << 8) | blue;
}

ColorRGBA LerpRGB(ColorRGBA c1, ColorRGBA c2, float t) {
    float r = c1.r + (c2.r - c1.r) * t;
    float g = c1.g + (c2.g - c1.g) * t;
    float b = c1.b + (c2.b - c1.b) * t;

    return {r, g, b, 1.0f};
}

float LerpFloat(float f1, float f2, float t) {
    float r = f1 + (f2 - f1) * t;

    return r;
}
