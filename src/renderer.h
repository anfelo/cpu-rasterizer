#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include <cstdint>

struct V2 {
    int x;
    int y;
};

class Renderer {
  public:
    uint32_t *pixels;
    int width, height;
    int scale; // Pixel scale factor

    Renderer(int w, int h, int pixelScale = 1);
    ~Renderer();

    int windowWidth() const {
        return width * scale;
    }
    int windowHeight() const {
        return height * scale;
    }

    void clear(uint32_t color = 0xFF000000);
    void setPixel(int x, int y, uint32_t color);
    void fillRect(int x, int y, int w, int h, uint32_t color);
    void drawCircle(int cx, int cy, int r, uint32_t color);
    void drawTriangles(float vertices[6], int num, uint32_t color);
    void drawLine(V2 p1, V2 p2, uint32_t color);
    void drawLineVertical(V2 p1, V2 p2, uint32_t color);
    void drawLineHorizontal(V2 p1, V2 p2, uint32_t color);
};

#endif
