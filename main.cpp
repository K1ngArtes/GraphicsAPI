#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue = TGAColor(0,   0,   255, 255);
const int width  = 200;
const int height = 200;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color);
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color);

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
	// Model model("obj/african_head.obj");
	// for(int i = 0; i < model.nfaces(); i++) {
	// 	std::vector<int> face = model.face(i);
	// 	for(int vId = 0; vId < 3; ++vId) {
	// 		Vec3f v0 = model.vertex(face[vId]);
	// 		Vec3f v1 = model.vertex(face[(vId+1)%3]);
	// 		int x0 = (v0.x+1.)*width/2.;
    //         int y0 = (v0.y+1.)*height/2.;
    //         int x1 = (v1.x+1.)*width/2.;
	// 		int y1 = (v1.y+1.)*height/2.;
	// 		line(x0, y0, x1, y1, image, white);
	// 	}
	// }

	Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle(t0[0], t0[1], t0[2], image, white);
    triangle(t1[0], t1[1], t1[2], image, white);
	triangle(t2[0], t2[1], t2[2], image, white);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {

	bool steep = false;
	int x0 = p0.x;
	int y0 = p0.y;
	int x1 = p1.x;
	int y1 = p1.y;

	if(std::abs(x0-x1) < std::abs(y0-y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if(x0>x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

    for (int x=x0; x<=x1; x++) { 
        float t = (x-x0)/(float)(x1 - x0);
        int y = y0*(1.-t) + y1*t;
		if(steep)
			image.set(y, x, color);
		else 
        	image.set(x, y, color); 
    } 
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {

	// t0, t1, t2 from lowest to highest
	if(t0.y > t1.y) std::swap(t0, t1);
	if(t0.y > t2.y) std::swap(t0, t2);
	if(t1.y > t2.y) std::swap(t1, t2);

	line(t0, t1, image, green);
	line(t1, t2, image, green);
	line(t0, t2, image, red);
}