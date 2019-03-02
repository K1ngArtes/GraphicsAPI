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
const int height = 800;
Vec3f lightdir(0, 0, -1);

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);
void triangle(Vec3f *pts, float *zBuffer, TGAImage &image, TGAColor color);
Vec3f world2Screen(const Vec3f &P);

int main(int argc, char **argv) {
    float *zBuffer = new float[width*height];

    for(int i = width*height; i > 0; i--) {
        zBuffer[i] = -std::numeric_limits<float>::max();
    }
    
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage texture;
    texture.read_tga_file("texture/african_head_diffuse.tga");
    Model model("obj/african_head.obj");

    for (int i = 0; i < model.nfaces(); i++) {
        Vec3f screenCoords[3];
        Vec3f woorldCoord[3];
        std::vector<int> face = model.face(i);
        for (int i = 0; i < 3; i++) {
            woorldCoord[i] = model.vertex(face[i]);
            screenCoords[i] = world2Screen(woorldCoord[i]);
        }
        // calculate light intensity
        Vec3f orthogonal = (woorldCoord[2]-woorldCoord[0]) ^ (woorldCoord[1]-woorldCoord[0]);
        orthogonal.normalize();
        float intensity = orthogonal*lightdir;
        if(intensity > 0)
            triangle(screenCoords, zBuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
    }

    image.flip_vertically();
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
        int y = y0+(y1-y0)*t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[3];
    for (int i=2; i--; ) {
        s[i].raw[0] = C.raw[i]-A.raw[i];
        s[i].raw[1] = B.raw[i]-A.raw[i];
        s[i].raw[2] = A.raw[i]-P.raw[i];
}
    // orthogonal is (u, v, 1) and is perpendicular to both (ABx, ACx, Px) and (ABy, ACy, Py)
    Vec3f orthogonal = s[0] ^ s[1];
    // if w is more than 1, then point is out of triangle
    if (std::abs(orthogonal.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1 - (orthogonal.x + orthogonal.y) / orthogonal.z,
                 orthogonal.y / orthogonal.z, orthogonal.x / orthogonal.z);
}

void triangle(Vec3f *pts, float *zBuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin.raw[j] = std::max(0.f, std::min(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = std::min(clamp.raw[j], std::max(bboxmax.raw[j], pts[i].raw[j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i].raw[2]*bc_screen.raw[i];
            if (zBuffer[int(P.x + P.y * width)] < P.z) {
                zBuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2Screen(const Vec3f &P) {
    return Vec3f(
        int((P.x + 1) * width/2.0 + 0.5),
        int((P.y + 1) * height/2.0 + 0.5),
        P.z);
}