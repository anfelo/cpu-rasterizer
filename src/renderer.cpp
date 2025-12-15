#include "renderer.h"
#include <cmath>

Renderer::Renderer(int w, int h, int pixelScale) : width(w), height(h), scale(pixelScale) {
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

void Renderer::drawTriangles(float vertices[6], int num, uint32_t color) {
    for (int i = 0; i < num; i += 2) {
        float p1x = vertices[i];
        float p1y = vertices[i + 1];

        for (int j = i + 2; j < num; j += 2) {
            float p2x = vertices[j];
            float p2y = vertices[j + 1];

            drawLine(p1x, p1y, p2x, p2y, color);
        }
    }
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer::drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    if (abs(y2 - y1) < abs(x2 - x1)) {
        if (x1 > x2) {
            drawLineHorizontal(x2, y2, x1, y1, color);
        } else {
            drawLineHorizontal(x1, y1, x2, y2, color);
        }
    } else {
        if (y1 > y2) {
            drawLineVertical(x2, y2, x1, y1, color);
        } else {
            drawLineVertical(x1, y1, x2, y2, color);
        }
    }
}

void Renderer::drawLineHorizontal(int x1, int y1, int x2, int y2,
                                  uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int d = (2 * dy) - dx;
    int y = y1;

    for (int x = x1; x < x2; ++x) {
        setPixel(x, y, color);

        if (d > 0) {
            y += yi;
            d = d + (2 * (dy - dx));
        } else {
            d = d + 2 * dy;
        }
    }
}

void Renderer::drawLineVertical(int x1, int y1, int x2, int y2,
                                uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int d = (2 * dx) - dy;
    int x = x1;

    for (int y = y1; y < y2; ++y) {
        setPixel(x, y, color);

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}
