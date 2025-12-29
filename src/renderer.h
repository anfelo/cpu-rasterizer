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

struct Pixel {
    int x;
    int y;
    float depth;
    ColorRGBA color;
    Vec3 normal;
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
void Renderer_FillTriangle(Renderer *r, std::vector<Pixel> *points);
void Renderer_DrawLine(Renderer *r, std::vector<Pixel> *points, Pixel p1,
                       Pixel p2);
void Renderer_DrawLineVertical(Renderer *r, std::vector<Pixel> *points,
                               Pixel p1, Pixel p2);
void Renderer_DrawLineHorizontal(Renderer *r, std::vector<Pixel> *points,
                                 Pixel p1, Pixel p2);

ColorRGBA CalculatePixelLighting(Pixel pixel);

CubeMesh CreateCubeMesh();
QuadMesh CreateQuadMesh();

#endif
