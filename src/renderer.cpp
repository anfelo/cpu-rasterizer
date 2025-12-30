#include "renderer.h"
#include "math.h"
#include <algorithm>
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
        // Some far value that acts as a reset
        r->zBuffer[i] = 1.0f;
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

void Renderer_DrawTriangles(Renderer *r, float *vertices, int length, int size,
                            Vec3 position, Vec3 rotation, Vec3 scale,
                            ColorRGBA color) {
    if (r == nullptr) {
        return;
    }

    if (vertices == NULL) {
        return;
    }

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

    std::vector<Pixel> pixels;

    // Vertices are in local space
    for (int i = 0; i < length * size; i += (size * 3)) {
        pixels.clear();

        int v1i = i;
        int v2i = i + size;
        int v3i = i + (size * 2);

        Vec4 v1 = {vertices[v1i], vertices[v1i + 1], vertices[v1i + 2], 1.0f};
        Vec4 v2 = {vertices[v2i], vertices[v2i + 1], vertices[v2i + 2], 1.0f};
        Vec4 v3 = {vertices[v3i], vertices[v3i + 1], vertices[v3i + 2], 1.0f};

        // Model -> World
        v1 = Vec4_Transform(v1, model);
        v2 = Vec4_Transform(v2, model);
        v3 = Vec4_Transform(v3, model);

        // World -> View
        v1 = Vec4_Transform(v1, view);
        v2 = Vec4_Transform(v2, view);
        v3 = Vec4_Transform(v3, view);

        // View -> Clip (Projection)
        v1 = Vec4_Transform(v1, projection);
        v2 = Vec4_Transform(v2, projection);
        v3 = Vec4_Transform(v3, projection);

        // TODO: Handle the offscreen vertices later on the draw call
        // if (v.w <= 0.0f) {
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

        // Rendering
        ColorRGBA p1Color = color;
        ColorRGBA p2Color = color;
        ColorRGBA p3Color = color;

        Vec3 p1Norm = {vertices[v1i + 3], vertices[v1i + 4], vertices[v1i + 5]};
        Vec3 p2Norm = {vertices[v2i + 3], vertices[v2i + 4], vertices[v2i + 5]};
        Vec3 p3Norm = {vertices[v3i + 3], vertices[v3i + 4], vertices[v3i + 5]};

        Pixel p1 = {(int)v1.x, (int)v1.y, v1.z, p1Color, p1Norm};
        Pixel p2 = {(int)v2.x, (int)v2.y, v2.z, p2Color, p2Norm};
        Pixel p3 = {(int)v3.x, (int)v3.y, v3.z, p3Color, p3Norm};

        // Generate all the pixels (fragments)
        Renderer_DrawLine(r, &pixels, p1, p2);
        Renderer_DrawLine(r, &pixels, p1, p3);
        Renderer_DrawLine(r, &pixels, p2, p3);

        Renderer_FillTriangle(r, &pixels);
    }
}

void Renderer_FillTriangle(Renderer *r, std::vector<Pixel> *pixels) {
    if (r == nullptr) {
        return;
    }

    // Sort by y component
    std::sort(pixels->begin(), pixels->end(),
              [](const Pixel &a, const Pixel &b) { return a.y < b.y; });

    for (size_t i = 0; i < pixels->size() - 1; ++i) {
        Pixel p1 = pixels->at(i);
        Pixel p2 = pixels->at(i + 1);

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
                float depth = LerpFloat(p1.depth, p2.depth,
                                        (float)(x - p1.x) / (p2.x - p1.x));

                Vec3 pNorm = {p1.normal.x, p1.normal.y, p1.normal.z};
                Pixel p = {x, p1.y, depth, colorRGB, pNorm};
                // colorRGB = Renderer_CalculatePixelLighting(r, p);

                Renderer_SetPixel(r, p.x, p.y, p.depth, ColorToInt(colorRGB));

                if (x != p1.x && x != p2.x) {
                    pixels->push_back(p);
                }
            }
        } else {
            for (int x = p2.x; x <= p1.x; ++x) {
                ColorRGBA colorRGB = LerpRGB(p2.color, p1.color,
                                             (float)(x - p2.x) / (p1.x - p2.x));
                float depth = LerpFloat(p2.depth, p1.depth,
                                        (float)(x - p2.x) / (p1.x - p2.x));

                Vec3 pNorm = {p1.normal.x, p1.normal.y, p1.normal.z};
                Pixel p = {x, p2.y, depth, colorRGB, pNorm};
                // colorRGB = Renderer_CalculatePixelLighting(r, p);

                Renderer_SetPixel(r, p.x, p.y, p.depth, ColorToInt(colorRGB));

                if (x != p1.x && x != p2.x) {
                    pixels->push_back(p);
                }
            }
        }
    }
}

// Bresenham's Line algorithm
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void Renderer_DrawLine(Renderer *r, std::vector<Pixel> *pixels, Pixel p1,
                       Pixel p2) {
    if (r == nullptr) {
        return;
    }

    if (abs(p2.y - p1.y) < abs(p2.x - p1.x)) {
        if (p1.x > p2.x) {
            Renderer_DrawLineHorizontal(r, pixels, p2, p1);
        } else {
            Renderer_DrawLineHorizontal(r, pixels, p1, p2);
        }
    } else {
        if (p1.y > p2.y) {
            Renderer_DrawLineVertical(r, pixels, p2, p1);
        } else {
            Renderer_DrawLineVertical(r, pixels, p1, p2);
        }
    }
}

void Renderer_DrawLineHorizontal(Renderer *r, std::vector<Pixel> *pixels,
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
        float depth = LerpFloat(p1.depth, p2.depth, (float)(x - p1.x) / dx);

        // INFO: For now we deal with flat faces so we keep the same normal as
        // one of the points. Maybe I need to interpolate the vectors?
        Vec3 pNorm = {p1.normal.x, p1.normal.y, p1.normal.z};

        pixels->push_back(Pixel{x, y, depth, colorRGB, pNorm});

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
        float depth = LerpFloat(p1.depth, p2.depth, (float)(y - p1.y) / dy);

        // INFO: For now we deal with flat faces so we keep the same normal as
        // one of the points. Maybe I need to interpolate the vectors?
        Vec3 pNorm = {p1.normal.x, p1.normal.y, p1.normal.z};

        points->push_back(Pixel{x, y, depth, colorRGB, pNorm});

        if (d > 0) {
            x += xi;
            d = d + (2 * (dx - dy));
        } else {
            d = d + 2 * dx;
        }
    }
}

ColorRGBA Renderer_CalculatePixelLighting(Renderer *r, Pixel pixel) {
    // Ambient
    float ambientStrength = 0.1f;
    Vec3 lightColor = {1.0f, 1.0f, 1.0f};
    Vec3 ambient = Vec3_ScalarMult(lightColor, ambientStrength);

    Vec3 pixelPos = {(float)pixel.x, (float)pixel.y, pixel.depth};

    // Diffuse
    Vec3 lightPos = {80.0f, 80.0f, 50.0f};
    Vec3 norm = Vec3_Normalize(pixel.normal);
    Vec3 lightDir = Vec3_Normalize(Vec3_Subtract(lightPos, pixelPos));
    float dp = Vec3_Dot(norm, lightDir);
    float diff = dp > 0.0 ? dp : 0.0;
    Vec3 diffuse = Vec3_ScalarMult(lightColor, diff);

    // Specular
    float specularStrength = 0.5f;
    Vec3 viewDir = Vec3_Normalize(Vec3_Subtract(r->camera.position, pixelPos));
    Vec3 reflectDir = Vec3_Reflect(Vec3_ScalarMult(lightDir, -1), norm);
    float spec = powf(fmaxf(Vec3_Dot(viewDir, reflectDir), 0.0), 32);
    Vec3 specular = Vec3_ScalarMult(lightColor, specularStrength * spec);

    Vec3 lighting = Vec3_Add(ambient, diffuse);
    lighting = Vec3_Add(lighting, specular);

    return {
        pixel.color.r * lighting.x,
        pixel.color.g * lighting.y,
        pixel.color.b * lighting.z,
        pixel.color.a,
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
