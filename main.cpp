#include <cstdlib>
#include <iostream>
#include <vector>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 500;
Vec3f lightdir(0, 0, -1);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void triangle(Vec2i *pts, TGAImage &image, TGAColor color);
void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]);

int main(int argc, char **argv) {
    TGAImage scene(width, height, TGAImage::RGB);

    // scene "2d mesh"
    line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
    line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
    line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

    // screen line
    line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

    scene.flip_vertically();  // i want to have the origin at the left bottom corner of the scene
    scene.write_tga_file("scene.tga");

    TGAImage render(width, 16, TGAImage::RGB);
    int yBuffer[width];

    //setup buffer with initial values
    for (int i = 0; i < width; i++) {
        yBuffer[i] = std::numeric_limits<int>::min();
    }
    rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, yBuffer);
    rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, yBuffer);
    rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, yBuffer);

    // 1-pixel wide image is bad for eyes, lets widen it
    for (int i = 0; i < width; i++) {
        for (int j = 1; j < 16; j++) {
            render.set(i, j, render.get(i, 0));
        }
    }
    
    render.flip_vertically();
    render.write_tga_file("render.tga");

    return 0;
}

void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int yBuffer[]) {
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }
    for (int x = p0.x; x <= p1.x; x++) {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y + t * (p1.y - p0.y);
        if (yBuffer[x] < y) {
            yBuffer[x] = y;
            image.set(x, 0, color);
        }
    }
}

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    int x0 = p0.x;
    int y0 = p0.y;
    int x1 = p1.x;
    int y1 = p1.y;

    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0+(y1-y0)*t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y == t1.y && t1.y == t2.y) return;
    // t0, t1, t2 from lowest to highest
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);

    int totalHeight = t2.y - t0.y;
    int firstHalfSize = t1.y - t0.y;
    int secondHalfSize = t2.y - t1.y;
    Vec2i t0t2 = t2 - t0;
    Vec2i t0t1 = t1 - t0;
    Vec2i t1t2 = t2 - t1;

    for (int i = 0; i < totalHeight; i++) {
        bool secondHalf = i > firstHalfSize;
        int segmentHeight = secondHalf ? secondHalfSize : firstHalfSize;
        float alpha = (float)i / totalHeight;
        float beta =
            (float)(i - (secondHalf ? firstHalfSize : 0)) / segmentHeight;
        Vec2i A = t0 + t0t2 * alpha;
        Vec2i B = secondHalf ? t1 + t1t2 * beta : t0 + t0t1 * beta;
        if (A.x > B.x) std::swap(A, B);
        for (int x = A.x; x <= B.x; x++) {
            image.set(x, t0.y + i, color);
        }
    }
}

Vec3f barycentric(Vec2i *pts, Vec2i P) {
    // orthogonal is (u, v, 1) and is perpendicular to both (ABx, ACx, Px) and (ABy, ACy, Py)
    Vec3f orthogonal =
        Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x)
            .cross(Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y));
    // if w is more than 1, then point is out of triangle
    if (std::abs(orthogonal.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1 - (orthogonal.x + orthogonal.y) / orthogonal.z,
                 orthogonal.y / orthogonal.z, orthogonal.x / orthogonal.z);
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color) {
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin.raw[j] = std::max(0, std::min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = std::min(clamp.raw[j], std::max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            image.set(P.x, P.y, color);
        }
    }
}