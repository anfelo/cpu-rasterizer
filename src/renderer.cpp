#include "renderer.h"
#include "math.h"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <vector>

Renderer Renderer_Create(int w, int h, int pixelScale) {
    Renderer r = {.width = w,
                  .height = h,
                  .windowWidth = w * pixelScale,
                  .windowHeight = h * pixelScale,
                  .pixelScale = pixelScale};

    r.pixels = new uint32_t[w * h];

    r.ready = true;

    return r;
}

void Renderer_Destroy(Renderer *r) {
    if (r == nullptr) {
        return;
    }

    delete[] r->pixels;
}

void Renderer_ClearBackground(Renderer *r, uint32_t color) {
    if (r == nullptr) {
        return;
    }

    for (int i = 0; i < r->width * r->height; i++) {
        r->pixels[i] = color;
    }
}

void Renderer_SetPixel(Renderer *r, int x, int y, uint32_t color) {
    if (r == nullptr) {
        return;
    }

    if (x >= 0 && x < r->width && y >= 0 && y < r->height) {
        r->pixels[y * r->width + x] = color;
    }
}

void Renderer_DrawTriangles(Renderer *r, float *vertices, int length, int size,
                            Vec3 position, Vec3 rotation) {
    if (r == nullptr) {
        return;
    }

    if (vertices == NULL) {
        return;
    }

    // Transformations
    Mat4 view = Mat4_Create();
    view = Mat4_Translate(view, Vec3{0.0f, 0.0f, -3.0f});
    Mat4 projection = Mat4_Perspective(
        DegToRadians(45.0f), (float)r->width / r->height, 0.1f, 100.0f);
    // Mat4 projection = Mat4_Ortho(-1.0f, 1.0f, 3.0f/4.0f, -3.0f/4.0f, 0.1f,
    // 100.0f);

    Mat4 model = Mat4_Create();
    model = Mat4_Rotate(model, rotation);

    // Vertices are in local space (NDC Coordinates)
    for (int i = 0; i < length * size; i += size) {
        Vec4 v = {vertices[i], vertices[i + 1], vertices[i + 2], 1.0f};
        // Model -> World
        v = Vec4_Transform(v, model);
        // World -> View
        v = Vec4_Transform(v, view);
        // View -> Clip (Projection)
        v = Vec4_Transform(v, projection);

        // Clip space
        vertices[i] = v.x;
        vertices[i + 1] = v.y;
        vertices[i + 2] = v.z;

        // Clip -> NDC (Perspective Divide)
        vertices[i] = vertices[i] / v.w;
        vertices[i + 1] = vertices[i + 1] / v.w;
        vertices[i + 2] = vertices[i + 2] / v.w;

        // NDC -> Screen
        vertices[i] = ((float)r->width / 2) * (vertices[i] + 1);
        vertices[i + 1] = ((float)r->height / 2) * (vertices[i + 1] + 1);
        vertices[i + 2] = ((float)r->height / 2) * (vertices[i + 2] + 1);
    }

    std::vector<Pixel> edge_points;

    // Rendering
    for (int i = 0; i < length * size; i += (size * 3)) {
        int p1i = i;
        int p2i = i + size;
        int p3i = i + (size * 2);

        ColorRGBA p1Color = {1.0f, 0.0f, 0.0f};
        ColorRGBA p2Color = {1.0f, 0.0f, 0.0f};
        ColorRGBA p3Color = {1.0f, 0.0f, 0.0f};

        Pixel p1 = {(int)vertices[p1i], (int)vertices[p1i + 1], p1Color};
        Pixel p2 = {(int)vertices[p2i], (int)vertices[p2i + 1], p2Color};
        Pixel p3 = {(int)vertices[p3i], (int)vertices[p3i + 1], p3Color};

        Renderer_DrawLine(r, &edge_points, p1, p2);
        Renderer_DrawLine(r, &edge_points, p1, p3);
        Renderer_DrawLine(r, &edge_points, p2, p3);
    }

    Renderer_FillTriangle(r, &edge_points);
}

void Renderer_FillTriangle(Renderer *r, std::vector<Pixel> *points) {
    if (r == nullptr) {
        return;
    }

    // Sort by y component
    std::sort(points->begin(), points->end(),
              [](const Pixel &a, const Pixel &b) { return a.y < b.y; });

    for (size_t i = 0; i < points->size() - 1; ++i) {
        Pixel p1 = points->at(i);
        Pixel p2 = points->at(i + 1);

        if (p1.y != p2.y) {
            continue;
        }

        if (p1.x == p2.x) {
            continue;
        }

        if (p1.x < p2.x) {
            for (int x = p1.x; x <= p2.x; ++x) {
                ColorRGBA colorRGB = LerpRGB(p1.color, p2.color,
                                             (float)(x - p1.x) / (p2.x - p1.x));

                Renderer_SetPixel(r, x, p1.y, ColorToInt(colorRGB));
            }
        } else {
            for (int x = p2.x; x <= p1.x; ++x) {
                ColorRGBA colorRGB = LerpRGB(p2.color, p1.color,
                                             (float)(x - p2.x) / (p1.x - p2.x));

                Renderer_SetPixel(r, x, p1.y, ColorToInt(colorRGB));
            }
        }
    }
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer_DrawLine(Renderer *r, std::vector<Pixel> *points, Pixel p1,
                       Pixel p2) {
    if (r == nullptr) {
        return;
    }

    if (abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            Renderer_DrawLineHorizontal(r, points, p2, p1);
        } else {
            Renderer_DrawLineHorizontal(r, points, p1, p2);
        }
    } else {
        if (p1.y > p2.y) {
            Renderer_DrawLineVertical(r, points, p2, p1);
        } else {
            Renderer_DrawLineVertical(r, points, p1, p2);
        }
    }
}

void Renderer_DrawLineHorizontal(Renderer *r, std::vector<Pixel> *points,
                                 Pixel p1, Pixel p2) {
    if (r == nullptr) {
        return;
    }

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int d = (2 * dy) - dx;
    int y = p1.y;

    for (int x = p1.x; x <= p2.x; ++x) {
        ColorRGBA colorRGB =
            LerpRGB(p1.color, p2.color, (float)(x - p1.x) / dx);

        Renderer_SetPixel(r, x, y, ColorToInt(colorRGB));

        points->push_back(Pixel{x, y, colorRGB});

        if (d > 0) {
            y += yi;
            d = d + (2 * (dy - dx));
        } else {
            d = d + 2 * dy;
        }
    }
}

void Renderer_DrawLineVertical(Renderer *r, std::vector<Pixel> *points,
                               Pixel p1, Pixel p2) {
    if (r == nullptr) {
        return;
    }

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int d = (2 * dx) - dy;
    int x = p1.x;

    for (int y = p1.y; y <= p2.y; ++y) {
        ColorRGBA colorRGB =
            LerpRGB(p1.color, p2.color, (float)(y - p1.y) / dy);

        Renderer_SetPixel(r, x, y, ColorToInt(colorRGB));

        points->push_back(Pixel{x, y, colorRGB});

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}
