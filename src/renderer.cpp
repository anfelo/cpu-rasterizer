#include "renderer.h"

Renderer::Renderer(int w, int h) : width(w), height(h) {
    pixels = new uint32_t[w * h];
    clear();
}

Renderer::~Renderer() {
    delete[] pixels;
}

void Renderer::clear(uint32_t color) {
    for (int i = 0; i < width * height; i++) {
        pixels[i] = color;
    }
}

void Renderer::setPixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        pixels[y * width + x] = color;
    }
}

void Renderer::fillRect(int x, int y, int w, int h, uint32_t color) {
    for (int py = y; py < y + h; py++) {
        for (int px = x; px < x + w; px++) {
            setPixel(px, py, color);
        }
    }
}

void Renderer::drawCircle(int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                setPixel(cx + x, cy + y, color);
            }
        }
    }
}
