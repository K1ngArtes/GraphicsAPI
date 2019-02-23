#include <iostream>
#include <vector>
#include <cstdlib>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 700;
const int height = 700;
int number = 0;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);
void triangle(Vec2i *pts, TGAImage &image, TGAColor color);

int main(int argc, char **argv) {
    TGAImage image(width, height, TGAImage::RGB);
    Model model("obj/african_head.obj");
    for (int i = 0; i < model.nfaces(); i++) {
        Vec2i screenCoords[3];
        std::vector<int> face = model.face(i);
        for (int vId = 0; vId < 3; ++vId) {
			Vec3f woorldCoord = model.vertex(face[vId]);
            screenCoords[vId] = Vec2i((woorldCoord.x+1.0)*width/2.0, (woorldCoord.y+1.0)*height/2.0);
        }
		triangle(screenCoords, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    image.flip_vertically();  // i want to have the origin at the left bottom
                              // corner of the image
    image.write_tga_file("output.tga");

    return 0;
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
        int y = y0 * (1. - t) + y1 * t;
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
    // orthogonal is (u, v, 1) and is perpendicular to both (ABx, ACx, Px) and
    // (ABy, ACy, Py)
    Vec3f orthogonal =
        Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x)
            .cross(Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y,
                         pts[0].y - P.y));
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
            bboxmin.raw[j] =
                std::max(0, std::min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] =
                std::min(clamp.raw[j], std::max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    std::cout << "Max: (" << bboxmax.x << ", " << bboxmax.y << ")" << std::endl;
    std::cout << "Min: (" << bboxmin.x << ", " << bboxmin.y << ")" << std::endl;
    Vec2i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts, P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            image.set(P.x, P.y, color);
        }
    }
}