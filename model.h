#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
    public:
        Model(const char *fileName);
        ~Model();
        int nverts();
        int nfaces();
        Vec3f vertex(int i);
        Vec2i uv(int iface, int nvert);
        std::vector<int> face(int i);
        std::vector<int> uvs(int i);
    private:
        std::vector<Vec3f > vertices_;
        std::vector<Vec3f > vertexTexture_;
        std::vector<Vec3f > uv_;
        std::vector<std::vector<int> > faces_;
        std::vector<std::vector<int> > uvs_;
        TGAImage diffusemap_;
        void loadTexture(std::string filename, const char *suffix, TGAImage &img);
        TGAColor diffuse(Vec2i uv);
};

#endif