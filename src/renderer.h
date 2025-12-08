#ifndef RASTERIZER_H_
#define RASTERIZER_H_

#include <cstdint>

class Renderer {
  public:
    uint32_t *pixels;
    int width, height;

    Renderer(int w, int h);
    ~Renderer();

    void clear(uint32_t color = 0xFF000000);
    void setPixel(int x, int y, uint32_t color);
    void fillRect(int x, int y, int w, int h, uint32_t color);
    void drawCircle(int cx, int cy, int r, uint32_t color);
};

#endif
