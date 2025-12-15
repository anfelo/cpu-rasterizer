#include "renderer.h"
#include <cmath>

Renderer::Renderer(int w, int h, int pixelScale)
    : width(w), height(h), scale(pixelScale) {
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
        V2 p1 = {(int)vertices[i], (int)vertices[i + 1]};

        for (int j = i + 2; j < num; j += 2) {
            V2 p2 = {(int)vertices[j], (int)vertices[j + 1]};

            drawLine(p1, p2, color);
        }
    }
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer::drawLine(V2 p1, V2 p2, uint32_t color) {
    if (abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            drawLineHorizontal(p2, p1, color);
        } else {
            drawLineHorizontal(p1, p2, color);
        }
    } else {
        if (p1.y > p2.y) {
            drawLineVertical(p2, p1, color);
        } else {
            drawLineVertical(p1, p2, color);
        }
    }
}

void Renderer::drawLineHorizontal(V2 p1, V2 p2, uint32_t color) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int d = (2 * dy) - dx;
    int y = p1.y;

    for (int x = p1.x; x < p2.x; ++x) {
        setPixel(x, y, color);

        if (d > 0) {
            y += yi;
            d = d + (2 * (dy - dx));
        } else {
            d = d + 2 * dy;
        }
    }
}

void Renderer::drawLineVertical(V2 p1, V2 p2, uint32_t color) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int d = (2 * dx) - dy;
    int x = p1.x;

    for (int y = p1.y; y < p2.y; ++y) {
        setPixel(x, y, color);

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}
