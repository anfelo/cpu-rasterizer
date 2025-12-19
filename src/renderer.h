#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include <cstdint>
#include <vector>

struct V2 {
    int x;
    int y;
};

struct Renderer {
    bool ready;
    uint32_t *pixels;
    int width, height;
    int windowWidth, windowHeight;
    int pixelScale;
};

Renderer Renderer_Create(int w, int h, int pixelScale = 1);
void Renderer_Destroy(Renderer *R);
void Renderer_ClearBackground(Renderer *R, uint32_t color = 0xFF000000);
void Renderer_SetPixel(Renderer *R, int x, int y, uint32_t color);
void Renderer_DrawTriangles(Renderer *R, float *vertices, int num,
                            uint32_t color);
void Renderer_FillTriangle(Renderer *R, std::vector<V2> *points,
                           uint32_t color);
void Renderer_DrawLine(Renderer *R, std::vector<V2> *points, V2 p1, V2 p2,
                       uint32_t color);
void Renderer_DrawLineVertical(Renderer *R, std::vector<V2> *points, V2 p1,
                               V2 p2, uint32_t color);
void Renderer_DrawLineHorizontal(Renderer *R, std::vector<V2> *points, V2 p1,
                                 V2 p2, uint32_t color);

#endif
