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
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float *zBuffer, float intensity);
Vec3i world2Screen(const Vec3f &P);

Model *model = NULL;
float *zBuffer = NULL;

int main(int argc, char **argv) {
    zBuffer = new float[width*height];

    for(int i = width*height; i > 0; i--) {
        zBuffer[i] = -std::numeric_limits<float>::max();
    }
    
    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("../obj/african_head.obj");

    for (int i = 0; i < model->nfaces(); i++) {
        Vec3i screenCoords[3];
        Vec3f worldCoord[3];
        std::vector<int> face = model->face(i);
        for (int i = 0; i < 3; i++) {
            worldCoord[i] = model->vertex(face[i]);
            screenCoords[i] = world2Screen(worldCoord[i]);
        }
        // calculate light intensity
        Vec3f orthogonal = (worldCoord[2]-worldCoord[0]) ^ (worldCoord[1]-worldCoord[0]);
        orthogonal.normalize();
        float intensity = orthogonal*lightdir;
        Vec2i uv[3];
        for(int k = 0; k < 3; k++)
            uv[k] = model->textureCoord(i, k);
        if(intensity > 0)
            triangle(screenCoords[0], screenCoords[1], screenCoords[2], uv[0], uv[1], uv[2], image, zBuffer, intensity);
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");

    { // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i=0; i<width; i++) {
            for (int j=0; j<height; j++) {
                zbimage.set(i, j, TGAColor(zBuffer[i+j*width], 1));
            }
        }
        zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        zbimage.write_tga_file("zbuffer.tga");
    }
    delete model;
    delete [] zBuffer;

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

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float *zBuffer, float intensity) {
    if (t0.y == t1.y && t0.y == t2.y)
    {   
        return;
    }
    // t0, t1, t2 from lowest to highest
    if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); }
    if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); }
    if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); }

    int totalHeight = t2.y - t0.y;
    int firstHalfSize = t1.y - t0.y;
    int secondHalfSize = t2.y - t1.y;
    Vec3i t0t2 = t2 - t0;
    Vec3i t0t1 = t1 - t0;
    Vec3i t1t2 = t2 - t1;

    Vec2i uv0uv2 = uv2 - uv0;
    Vec2i uv0uv1 = uv1 - uv0;
    Vec2i uv1uv2 = uv2 - uv1;

    // seg fault cause of segment height being 0
    // for (int i = 0; i < totalHeight; i++) {
    //     bool secondHalf = i > firstHalfSize;
    //     int segmentHeight = secondHalf ? secondHalfSize : firstHalfSize;
    //     if(segmentHeight <= 0) segmentHeight=1;
    //     float alpha = (float)i / totalHeight;
    //     float beta = (float)(i - (secondHalf ? firstHalfSize : 0)) / segmentHeight;
    //     Vec3i A =              t0 + t0t2 * alpha;
    //     Vec3i B = secondHalf ? t1 + t1t2 * beta : t0 + t0t1 * beta;
    //     std::cout << "Segment height: " << segmentHeight << std::endl;
    //     std::cout << "Alpha: " << alpha << ", Beta: " << beta << std::endl;
    //     std::cout << "Points are " << t0 << t1 << t2 << std::endl;
    //     std::cout << "A.x: " << A.x << ", B.x: " << B.x << std::endl;
    //     Vec2i uvA =              uv0 + uv0uv2 * alpha;
    //     Vec2i uvB = secondHalf ? uv1 + uv1uv2 * beta : uv0 + uv0uv1 * beta;
    //     if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); }
    //     for (int x = A.x; x <= B.x; x++) {
    //         // phi represents the ration moved on x-axis
    //         float phi = (x == B.x) ? 1.0 : (x - A.x) / (B.x - A.x);
    //         Vec3i P = A + (B - A) * phi;
    //         Vec2i uvP = uvA + (uvB - uvA) * phi;
    //         int idx = P.x + P.y * width;
    //         if(zBuffer[idx] < P.z) {
    //             zBuffer[idx] = P.z;
    //             TGAColor c = model->diffuse(uvP);
    //             image.set(P.x, P.y, TGAColor(c.r * intensity, c.g * intensity, c.b * intensity, 255));
    //         }
    //         // std::cout << "Max value for int is " << std::numeric_limits<int>::max() << std::endl;
    //     }
    // }

    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;  // be careful: with above conditions no division by zero here
        Vec3i A = t0 + t0t2 * alpha;
        Vec3i B = second_half ? t1 + t1t2 * beta : t0 + t0t1 * beta;
        Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
        Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
        if (A.x > B.x) {
            std::swap(A, B);
            std::swap(uvA, uvB);
        }
        for (int j = A.x; j <= B.x; j++) {
            float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
            Vec3i P = (A) + (B - A) * phi;
            Vec2i uvP = uvA + (uvB - uvA) * phi;
            int idx = P.x + P.y * width;
            if (zBuffer[idx] < P.z) {
                zBuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, 255));
            }
        }
    }
}

Vec3i world2Screen(const Vec3f &P) {
    return Vec3i(
        int((P.x + 1) * width/2.0 + 0.5),
        int((P.y + 1) * height/2.0 + 0.5),
        P.z);
}