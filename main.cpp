#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color);

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
	// line(13, 20, 80, 40, image, white);
	// line(13, 20, 80, 0, image, white);  
	// line(20, 13, 40, 80, image, red); 
	// image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	// image.write_tga_file("output.tga");
	line(339, 353, 343, 300, image, red);
	Model model("obj/african_head.obj");
	for(int i = 0; i < model.nfaces(); i++) {
		std::vector<int> face = model.face(i);
		for(int vId = 0; vId < 3; ++vId) {
			Vec3f v0 = model.vertex(face[vId]);
			Vec3f v1 = model.vertex(face[(vId+1)%3]);
			int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
			int y1 = (v1.y+1.)*height/2.;
			line(x0, y0, x1, y1, image, white);
		}
	}
	// image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}


// this is unoptimised but working code
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
	bool steep = false;
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