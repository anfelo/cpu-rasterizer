#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include "camera.h"
#include "math.h"
#include <cstdint>
#include <vector>

struct CubeMesh {
    float vertices[216];
    uint32_t numVertices;
    uint32_t vertexSize;
};

struct QuadMesh {
    float vertices[36];
    uint32_t numVertices;
    uint32_t vertexSize;
};

struct Fragment {
    Vec2 coords;
    Vec3 normal;
    float z;
    ColorRGBA color;
};

struct Vertex {
    Vec3 coords;
    Vec3 normal;
    ColorRGBA color;
};

struct Triangle {
    Vertex v0, v1, v2;
    Vec2 min, max;
    float area;
};

struct Renderer {
    bool ready;
    uint32_t *pixels;
    int width, height;
    int windowWidth, windowHeight;
    int pixelScale;

    float *zBuffer;

    Camera camera;
};

Renderer Renderer_Create(int w, int h, int pixelScale = 1);
void Renderer_Destroy(Renderer *r);
void Renderer_ClearBackground(Renderer *r, uint32_t color = 0xFF000000);
void Renderer_SetPixel(Renderer *r, int x, int y, float z, uint32_t color);
void Renderer_DrawQuad(Renderer *r, Vec3 position, Vec3 rotation, Vec3 scale,
                       ColorRGBA color);
void Renderer_DrawCube(Renderer *r, Vec3 position, Vec3 rotation, Vec3 scale,
                       ColorRGBA color);
void Renderer_DrawTriangles(Renderer *r, float *vertices, int length, int size,
                            Vec3 position, Vec3 rotation, Vec3 scale,
                            ColorRGBA color);
void Renderer_DrawTriangle(Renderer *r, Vec2 vertices[3], uint32_t color);
void Renderer_FillTriangle(Renderer *r, std::vector<Vec2> *points,
                           uint32_t color);
void Renderer_DrawLine(Renderer *r, std::vector<Vec2> *points, Vec2 p1, Vec2 p2,
                       uint32_t color);
void Renderer_DrawLineVertical(Renderer *r, std::vector<Vec2> *points, Vec2 p1,
                               Vec2 p2, uint32_t color);
void Renderer_DrawLineHorizontal(Renderer *r, std::vector<Vec2> *points,
                                 Vec2 p1, Vec2 p2, uint32_t color);

ColorRGBA Renderer_CalculateFragmentLighting(Renderer *r, Fragment frag);

CubeMesh CreateCubeMesh();
QuadMesh CreateQuadMesh();

#endif
