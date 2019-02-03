#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "geometry.h"

class Model {
    public:
        Model(const char *fileName);
        ~Model();
        int nverts();
        int nfaces();
        Vec3f vertex(int i);
        std::vector<int> face(int i);
    private:
        std::vector<Vec3f > vertices_;
        std::vector<Vec3f > vertexTexture_;
        std::vector<std::vector<int> > faces_;
};

#endif