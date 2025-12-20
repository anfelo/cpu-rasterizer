#ifndef MATH_H_
#define MATH_H_

const float PI = 3.141592653589793;

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

struct Mat4 {
    float data[4 * 4];
};

Mat4 Mat4_Create();
Mat4 Mat4_Translate(Mat4 mat4, Vec3 vec3);
Mat4 Mat4_Scale(Mat4 mat4, Vec3 vec3);
Mat4 Mat4_Perspective(float fovy, float aspect, float zNear, float zFar);
Mat4 Mat4_Ortho(float left, float right, float bottom, float top, float zNear,
                float zFar);

Vec4 Vec4_Transform(Vec4 vec4, Mat4 mat4);

float DegToRadians(float deg);

#endif
