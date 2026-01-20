#include "renderer.h"
#include "math.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <thread>
#include <vector>

Renderer Renderer_Create(int w, int h, int pixelScale) {
    Renderer r = {.width = w,
                  .height = h,
                  .windowWidth = w * pixelScale,
                  .windowHeight = h * pixelScale,
                  .pixelScale = pixelScale};

    r.pixels = new uint32_t[w * h];
    r.zBuffer = new float[w * h];

    r.ready = true;

    return r;
}

void Renderer_Destroy(Renderer *r) {
    if (r == nullptr) {
        return;
    }

    delete[] r->pixels;
    delete[] r->zBuffer;
}

void Renderer_ClearBackground(Renderer *r, uint32_t color) {
    if (r == nullptr) {
        return;
    }

    for (int i = 0; i < r->width * r->height; i++) {
        r->pixels[i] = color;
        r->zBuffer[i] = std::numeric_limits<float>::infinity();
    }
}

void Renderer_SetPixel(Renderer *r, int x, int y, float z, uint32_t color) {
    if (r == nullptr) {
        return;
    }

    if (x < 0 || x >= r->width || y < 0 || y >= r->height)
        return;

    int idx = y * r->width + x;

    if (z < r->zBuffer[idx]) {
        r->zBuffer[idx] = z;
        r->pixels[idx] = color;
    }
}

void Renderer_DrawCube(Renderer *r, Vec3 position, Vec3 rotation, Vec3 scale,
                       ColorRGBA color) {
    if (r == nullptr) {
        return;
    }

    CubeMesh mesh = CreateCubeMesh();

    Renderer_DrawTriangles(r, mesh.vertices, mesh.numVertices, mesh.vertexSize,
                           position, rotation, scale, color);
}

void Renderer_DrawQuad(Renderer *r, Vec3 position, Vec3 rotation, Vec3 scale,
                       ColorRGBA color) {
    if (r == nullptr) {
        return;
    }

    QuadMesh mesh = CreateQuadMesh();

    Renderer_DrawTriangles(r, mesh.vertices, mesh.numVertices, mesh.vertexSize,
                           position, rotation, scale, color);
}

bool TriangleIntersectsTile(Triangle triangle, int tileX0, int tileY0,
                            int tileX1, int tileY1) {
    return !(triangle.max.x < tileX0 || triangle.min.x > tileX1 ||
             triangle.max.y < tileY0 || triangle.min.y > tileY1);
}

float TriangleEdgeFunction(Vec3 a, Vec3 b, Vec2 p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

Fragment TriangleInterpolatePoint(Triangle triangle, float b0, float b1,
                                  float b2) {
    Fragment frag;

    // Perspective-correct interpolation
    float invZ0 = 1.0f / triangle.v0.coords.z;
    float invZ1 = 1.0f / triangle.v1.coords.z;
    float invZ2 = 1.0f / triangle.v2.coords.z;

    float invZ = b0 * invZ0 + b1 * invZ1 + b2 * invZ2;
    float z = 1.0f / invZ;

    // Frag Depth
    frag.z = z;

    // Frag Color
    frag.color.r =
        (b0 * triangle.v0.color.r * invZ0 + b1 * triangle.v1.color.r * invZ1 +
         b2 * triangle.v2.color.r * invZ2) *
        z;
    frag.color.g =
        (b0 * triangle.v0.color.g * invZ0 + b1 * triangle.v1.color.g * invZ1 +
         b2 * triangle.v2.color.g * invZ2) *
        z;
    frag.color.b =
        (b0 * triangle.v0.color.b * invZ0 + b1 * triangle.v1.color.b * invZ1 +
         b2 * triangle.v2.color.b * invZ2) *
        z;

    // Frag Normals
    frag.normal.x =
        (b0 * triangle.v0.normal.x * invZ0 + b1 * triangle.v1.normal.x * invZ1 +
         b2 * triangle.v2.normal.x * invZ2) *
        z;
    frag.normal.y =
        (b0 * triangle.v0.normal.y * invZ0 + b1 * triangle.v1.normal.y * invZ1 +
         b2 * triangle.v2.normal.y * invZ2) *
        z;
    frag.normal.z =
        (b0 * triangle.v0.normal.z * invZ0 + b1 * triangle.v1.normal.z * invZ1 +
         b2 * triangle.v2.normal.z * invZ2) *
        z;

    frag.normal = Vec3_Normalize(frag.normal);

    return frag;
}

void Renderer_RasterizeTile(Renderer *r, const std::vector<Triangle> &triangles,
                            int tileX, int tileY, int tileSize) {
    int x0 = tileX * tileSize;
    int y0 = tileY * tileSize;
    int x1 = std::min(x0 + tileSize, r->width);
    int y1 = std::min(y0 + tileSize, r->height);

    for (const auto &triangle : triangles) {
        // Triangle area (for barycentric coordinates)
        // Skip degenerate triangles
        if (std::abs(triangle.area) < 0.0001f) {
            return;
        }

        // Determine winding
        bool clockwise = triangle.area < 0;
        float invArea = 1.0f / triangle.area;

        if (!TriangleIntersectsTile(triangle, x0, y0, x1, y1)) {
            continue;
        }

        // Rasterize only within tile bounds
        for (int y = y0; y < y1; y++) {
            for (int x = x0; x < x1; x++) {
                Vec2 p = {x + 0.5f, y + 0.5f};

                float w0 = TriangleEdgeFunction(triangle.v1.coords,
                                                triangle.v2.coords, p);
                float w1 = TriangleEdgeFunction(triangle.v2.coords,
                                                triangle.v0.coords, p);
                float w2 = TriangleEdgeFunction(triangle.v0.coords,
                                                triangle.v1.coords, p);

                // Check inside based on winding
                bool inside = clockwise ? (w0 <= 0 && w1 <= 0 && w2 <= 0)
                                        : (w0 >= 0 && w1 >= 0 && w2 >= 0);

                if (inside) {
                    float b0 = w0 * invArea;
                    float b1 = w1 * invArea;
                    float b2 = w2 * invArea;

                    Fragment frag =
                        TriangleInterpolatePoint(triangle, b0, b1, b2);
                    frag.coords = {p.x, p.y};

                    ColorRGBA fragColor =
                        Renderer_CalculateFragmentLighting(r, frag);
                    frag.color = fragColor;

                    // Gamma Correction
                    // frag.color = ColorToSRGB(frag.color);

                    Renderer_SetPixel(r, frag.coords.x, frag.coords.y, frag.z,
                                      ColorRGBAToInt(frag.color));
                }
            }
        }
    }
}

void Renderer_RasterizeTriangles(Renderer *r,
                                 const std::vector<Triangle> &triangles) {
    for (const auto &triangle : triangles) {
        // Triangle area (for barycentric coordinates)
        // Skip degenerate triangles
        if (std::abs(triangle.area) < 0.0001f) {
            return;
        }

        // Determine winding
        bool clockwise = triangle.area < 0;
        float invArea = 1.0f / triangle.area;

        // Rasterize only within tile bounds
        for (int y = triangle.min.y; y <= triangle.max.y; y++) {
            for (int x = triangle.min.x; x <= triangle.max.x; x++) {
                Vec2 p = {x + 0.5f, y + 0.5f};

                float w0 = TriangleEdgeFunction(triangle.v1.coords,
                                                triangle.v2.coords, p);
                float w1 = TriangleEdgeFunction(triangle.v2.coords,
                                                triangle.v0.coords, p);
                float w2 = TriangleEdgeFunction(triangle.v0.coords,
                                                triangle.v1.coords, p);

                // Check inside based on winding
                bool inside = clockwise ? (w0 <= 0 && w1 <= 0 && w2 <= 0)
                                        : (w0 >= 0 && w1 >= 0 && w2 >= 0);

                if (inside) {
                    float b0 = w0 * invArea;
                    float b1 = w1 * invArea;
                    float b2 = w2 * invArea;

                    Fragment frag =
                        TriangleInterpolatePoint(triangle, b0, b1, b2);
                    frag.coords = {p.x, p.y};

                    ColorRGBA fragColor =
                        Renderer_CalculateFragmentLighting(r, frag);
                    frag.color = fragColor;

                    Renderer_SetPixel(r, frag.coords.x, frag.coords.y, frag.z,
                                      ColorRGBAToInt(frag.color));
                }
            }
        }
    }
}

void Renderer_DrawTriangles(Renderer *r, float *vertices, int length, int size,
                            Vec3 position, Vec3 rotation, Vec3 scale,
                            ColorRGBA color) {
    float halfWidth = (float)r->width / 2;
    float halfHeight = (float)r->height / 2;

    // Transformations
    Mat4 view = Camera_GetViewMatrix(&r->camera);
    // Mat4 view = Mat4_Create();
    // view = Mat4_Translate(view, {0.0f, 0.0f, -3.0f});
    Mat4 projection =
        Mat4_Perspective(DegToRadians(r->camera.zoom),
                         (float)r->width / r->height, 0.1f, 100.0f);

    Mat4 model = Mat4_Create();
    model = Mat4_Rotate(model, rotation);
    model = Mat4_Scale(model, scale);
    model = Mat4_Translate(model, position);

    std::vector<Triangle> triangles;

    // Vertices are in local space
    for (int i = 0; i < length * size; i += (size * 3)) {
        int v1i = i;
        int v2i = i + size;
        int v3i = i + (size * 2);

        Vec4 v1 = {vertices[v1i], vertices[v1i + 1], vertices[v1i + 2], 1.0f};
        Vec4 v2 = {vertices[v2i], vertices[v2i + 1], vertices[v2i + 2], 1.0f};
        Vec4 v3 = {vertices[v3i], vertices[v3i + 1], vertices[v3i + 2], 1.0f};

        Vec3 v1Norm = {vertices[v1i + 3], vertices[v1i + 4], vertices[v1i + 5]};
        Vec3 v2Norm = {vertices[v2i + 3], vertices[v2i + 4], vertices[v2i + 5]};
        Vec3 v3Norm = {vertices[v3i + 3], vertices[v3i + 4], vertices[v3i + 5]};

        // Model -> World
        v1 = Vec4_Transform(v1, Mat4_Transpose(model));
        v2 = Vec4_Transform(v2, Mat4_Transpose(model));
        v3 = Vec4_Transform(v3, Mat4_Transpose(model));

        // World -> View
        v1 = Vec4_Transform(v1, view);
        v2 = Vec4_Transform(v2, view);
        v3 = Vec4_Transform(v3, view);

        // View -> Clip (Projection)
        v1 = Vec4_Transform(v1, projection);
        v2 = Vec4_Transform(v2, projection);
        v3 = Vec4_Transform(v3, projection);

        // TODO: Handle the offscreen vertices later on the draw call
        // if (v1.w <= 0.0f) {
        //     // Offscreen marker
        //     vertices[i] = -1.0f;
        //     vertices[i + 1] = -1.0f;
        //     vertices[i + 2] = -1.0f;
        //     continue;
        // }

        // Clip -> NDC (Perspective Divide)
        v1.x = v1.x / v1.w;
        v1.y = v1.y / v1.w;
        v1.z = v1.z / v1.w;

        v2.x = v2.x / v2.w;
        v2.y = v2.y / v2.w;
        v2.z = v2.z / v2.w;

        v3.x = v3.x / v3.w;
        v3.y = v3.y / v3.w;
        v3.z = v3.z / v3.w;

        // NDC -> Screen
        v1.x = halfWidth * (v1.x + 1.0f);
        v1.y = halfHeight * (1.0f - v1.y);
        v1.z = (v1.z + 1.0f) * 0.5;

        v2.x = halfWidth * (v2.x + 1.0f);
        v2.y = halfHeight * (1.0f - v2.y);
        v2.z = (v2.z + 1.0f) * 0.5;

        v3.x = halfWidth * (v3.x + 1.0f);
        v3.y = halfHeight * (1.0f - v3.y);
        v3.z = (v3.z + 1.0f) * 0.5;

        ColorRGBA v1Color = color;
        ColorRGBA v2Color = color;
        ColorRGBA v3Color = color;

        Vec2 vMin = {
            (float)std::max(
                0, static_cast<int>(std::floor(std::min({v1.x, v2.x, v3.x})))),
            (float)std::max(
                0, static_cast<int>(std::floor(std::min({v1.y, v2.y, v3.y})))),
        };
        Vec2 vMax = {
            (float)std::min(r->width - 1, static_cast<int>(std::ceil(
                                              std::max({v1.x, v2.x, v3.x})))),
            (float)std::min(r->height - 1, static_cast<int>(std::ceil(
                                               std::max({v1.y, v2.y, v3.y})))),
        };

        Triangle triangle = {
            .v0 = Vertex{Vec3{v1.x, v1.y, v1.z}, v1Norm, v1Color},
            .v1 = Vertex{Vec3{v2.x, v2.y, v2.z}, v2Norm, v2Color},
            .v2 = Vertex{Vec3{v3.x, v3.y, v3.z}, v3Norm, v3Color},
            .min = vMin,
            .max = vMax,
            .area =
                TriangleEdgeFunction(Vec3{v1.x, v1.y, v1.z},
                                     Vec3{v2.x, v2.y, v2.z}, Vec2{v3.x, v3.y}),
        };

        triangles.push_back(triangle);
    }

    // Single-Tread
    // Renderer_RasterizeTriangles(r, triangles);

    // Rasterize Triangles Multi-Threat
    // Render by tiles 32x32
    const int tileSize = 32;
    int tilesX = (r->width + tileSize - 1) / tileSize;
    int tilesY = (r->height + tileSize - 1) / tileSize;

    std::vector<std::thread> threads;
    int numThreads = std::thread::hardware_concurrency();

    for (int t = 0; t < numThreads; t++) {
        threads.emplace_back([&, t]() {
            for (int i = t; i < tilesX * tilesY; i += numThreads) {
                int tx = i % tilesX;
                int ty = i / tilesX;
                Renderer_RasterizeTile(r, triangles, tx, ty, tileSize);
            }
        });
    }

    for (auto &th : threads) {
        th.join();
    }
}

void Renderer_FillTriangle(Renderer *r, std::vector<Vec2> *points,
                           uint32_t color) {
    if (r == nullptr) {
        return;
    }

    // Sort by y component
    std::sort(points->begin(), points->end(),
              [](const Vec2 &a, const Vec2 &b) { return a.y < b.y; });

    for (size_t i = 0; i < points->size() - 1; ++i) {
        Vec2 p1 = points->at(i);
        Vec2 p2 = points->at(i + 1);

        if (p1.y != p2.y) {
            continue;
        }

        if (p1.x == p2.x) {
            continue;
        }

        if (p1.x < p2.x) {
            for (int x = p1.x; x <= p2.x; ++x) {
                Renderer_SetPixel(r, x, p1.y, 0.0f, color);
            }
        } else {
            for (int x = p2.x; x <= p1.x; ++x) {
                Renderer_SetPixel(r, x, p1.y, 0.0f, color);
            }
        }
    }
}

void Renderer_DrawTriangle(Renderer *r, Vec2 vertices[3], uint32_t color) {
    Vec2 p1 = vertices[0];
    Vec2 p2 = vertices[1];
    Vec2 p3 = vertices[2];

    std::vector<Vec2> points;

    Renderer_DrawLine(r, &points, p1, p2, color);
    Renderer_DrawLine(r, &points, p2, p3, color);
    Renderer_DrawLine(r, &points, p1, p3, color);

    Renderer_FillTriangle(r, &points, color);
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer_DrawLine(Renderer *r, std::vector<Vec2> *points, Vec2 p1, Vec2 p2,
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

void Renderer_DrawLineHorizontal(Renderer *r, std::vector<Vec2> *points,
                                 Vec2 p1, Vec2 p2, uint32_t color) {
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
        Renderer_SetPixel(r, x, y, 0.0f, color);

        Vec2 p = {(float)x, (float)y};
        points->push_back(p);

        if (d > 0) {
            y += yi;
            d = d + (2 * (dy - dx));
        } else {
            d = d + 2 * dy;
        }
    }
}

void Renderer_DrawLineVertical(Renderer *r, std::vector<Vec2> *points, Vec2 p1,
                               Vec2 p2, uint32_t color) {
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
        Renderer_SetPixel(r, x, y, 0.0f, color);

        Vec2 p = {(float)x, (float)y};
        points->push_back(p);

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}

ColorRGBA Renderer_CalculateFragmentLighting(Renderer *r, Fragment frag) {
    // Ambient
    float ambientStrength = 0.1f;
    Vec3 lightColor = {1.0f, 1.0f, 1.0f};
    Vec3 ambient = Vec3_ScalarMult(lightColor, ambientStrength);

    Vec3 fragPos = {(float)frag.coords.x, (float)frag.coords.y, frag.z};

    // Diffuse
    Vec3 lightPos = {80.0f, 50.0f, 50.0f};
    Vec3 norm = Vec3_Normalize(frag.normal);
    Vec3 lightDir = Vec3_Normalize(Vec3_Subtract(lightPos, fragPos));
    float dp = Vec3_Dot(norm, lightDir);
    float diff = dp > 0.0 ? dp : 0.0;
    Vec3 diffuse = Vec3_ScalarMult(lightColor, diff);

    // Specular
    float specularStrength = 0.5f;
    Vec3 viewDir = Vec3_Normalize(Vec3_Subtract(r->camera.position, fragPos));
    Vec3 reflectDir = Vec3_Reflect(Vec3_ScalarMult(lightDir, -1), norm);
    float spec = powf(fmaxf(Vec3_Dot(viewDir, reflectDir), 0.0), 32);
    Vec3 specular = Vec3_ScalarMult(lightColor, specularStrength * spec);

    Vec3 lighting = Vec3_Add(ambient, diffuse);
    lighting = Vec3_Add(lighting, specular);

    return {
        frag.color.r * lighting.x,
        frag.color.g * lighting.y,
        frag.color.b * lighting.z,
        frag.color.a,
    };
}

CubeMesh CreateCubeMesh() {
    CubeMesh mesh = {
        .vertices{
            // Geometry + Normals
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f,
            0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, 0.5f,  -0.5f,
            0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,
            0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, 0.5f,  0.5f,
            0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
            -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, 0.5f,
            -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
            1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
            1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f,
            0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, 0.5f,
            0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
            0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,
            0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
        },
        .numVertices = 36,
        .vertexSize = 6,
    };

    return mesh;
}

QuadMesh CreateQuadMesh() {
    QuadMesh mesh = {
        .vertices{
            // Geometry + Normals
            -0.5f, -0.5f, 0.0f, 0.0f,  1.0f,  0.0f, 0.5f,  -0.5f, 0.0f,
            0.0f,  1.0f,  0.0f, 0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f, -0.5f, 0.5f,  0.0f,
            0.0f,  1.0f,  0.0f, -0.5f, -0.5f, 0.0f, 0.0f,  1.0f,  0.0f,
        },
        .numVertices = 6,
        .vertexSize = 6,
    };

    return mesh;
}
