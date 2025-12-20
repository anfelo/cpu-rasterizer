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

void Renderer_DrawTriangles(Renderer *r, float *vertices, int num,
                            Vec3 position, uint32_t color) {
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
    // Mat4 projection = Mat4_Ortho(-1.0f, 1.0f, 3.0f/4.0f, -3.0f/4.0f, 0.1f, 100.0f);

    Mat4 model = Mat4_Create();
    // model = Mat4_Translate(model, position);

    // Vertices are in local space (NDC Coordinates)
    float verts[num];
    for (int i = 0; i < num; i += 2) {
        Vec4 v = {vertices[i], vertices[i + 1], 0.0f, 1.0f};
        // Model -> World
        v = Vec4_Transform(v, model);
        // World -> View
        v = Vec4_Transform(v, view);
        // View -> Clip (Projection)
        v = Vec4_Transform(v, projection);

        // Clip space
        verts[i] = v.x;
        verts[i + 1] = v.y;

        // Clip -> NDC (Perspective Divide)
        verts[i] = verts[i] / v.w;
        verts[i + 1] = verts[i + 1] / v.w;

        // NDC -> Screen
        verts[i] = ((float)r->width / 2) * (verts[i] + 1);
        verts[i + 1] = ((float)r->height / 2) * (verts[i + 1] + 1);
    }

    std::vector<P2> edge_points;

    // Rendering
    for (int i = 0; i < num; i += 2) {
        P2 p1 = {(int)verts[i], (int)verts[i + 1]};

        for (int j = i + 2; j < num; j += 2) {
            P2 p2 = {(int)verts[j], (int)verts[j + 1]};

            Renderer_DrawLine(r, &edge_points, p1, p2, color);
        }
    }

    Renderer_FillTriangle(r, &edge_points, color);
}

void Renderer_FillTriangle(Renderer *r, std::vector<P2> *points,
                           uint32_t color) {
    if (r == nullptr) {
        return;
    }

    // Sort by y component
    std::sort(points->begin(), points->end(),
              [](const P2 &a, const P2 &b) { return a.y < b.y; });

    std::vector<P2> inner_points;
    for (size_t i = 0; i < points->size() - 1; ++i) {
        P2 p1 = points->at(i);
        P2 p2 = points->at(i + 1);

        if (p1.y != p2.y) {
            continue;
        }

        if (p1.x < p2.x) {
            for (int x = p1.x; x <= p2.x; ++x) {
                Renderer_SetPixel(r, x, p1.y, color);
            }
        } else {
            for (int x = p2.x; x <= p1.x; ++x) {
                Renderer_SetPixel(r, x, p1.y, color);
            }
        }
    }
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer_DrawLine(Renderer *r, std::vector<P2> *points, P2 p1, P2 p2,
                       uint32_t color) {
    if (r == nullptr) {
        return;
    }

    if (abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            Renderer_DrawLineHorizontal(r, points, p2, p1, color);
        } else {
            Renderer_DrawLineHorizontal(r, points, p1, p2, color);
        }
    } else {
        if (p1.y > p2.y) {
            Renderer_DrawLineVertical(r, points, p2, p1, color);
        } else {
            Renderer_DrawLineVertical(r, points, p1, p2, color);
        }
    }
}

void Renderer_DrawLineHorizontal(Renderer *r, std::vector<P2> *points, P2 p1,
                                 P2 p2, uint32_t color) {
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
        // DEBUG: Paint the vertices with a different color
        uint32_t c = (x == p1.x && y == p1.y) || (x == p2.x && y == p2.y)
                         ? 0xFFFF00
                         : color;
        Renderer_SetPixel(r, x, y, c);
        points->push_back(P2{x, y});

        if (d > 0) {
            y += yi;
            d = d + (2 * (dy - dx));
        } else {
            d = d + 2 * dy;
        }
    }
}

void Renderer_DrawLineVertical(Renderer *r, std::vector<P2> *points, P2 p1,
                               P2 p2, uint32_t color) {
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
        // DEBUG: Paint the vertices with a different color
        uint32_t c = (x == p1.x && y == p1.y) || (x == p2.x && y == p2.y)
                         ? 0xFFFF00
                         : color;
        Renderer_SetPixel(r, x, y, c);
        points->push_back(P2{x, y});

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}
