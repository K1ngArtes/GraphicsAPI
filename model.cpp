#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "model.h"

Model::Model(const char *fileName) : vertices_(), faces_()
{
    std::ifstream objFile(fileName);
    if (objFile.is_open())
    {
        std::string line;
        while (!objFile.eof())
        {
            std::getline(objFile, line);
            std::istringstream iss(line.c_str());
            char trash;
            std::string vertex;
            if (!line.compare(0, 2, "v "))
            {
                iss >> trash;
                Vec3f v;
                for (int i = 0; i < 3; i++)
                {
                    iss >> vertex;
                    v.raw[i] = std::stof(vertex);
                }
                vertices_.push_back(v);
            } else if (!line.compare(0, 2, "f ")) {
                std::vector<int> f;
                std::vector<int> uvPos;
                int itrash, idx, uv;
                iss >> trash;
                while (iss >> idx >> trash >> uv >> trash >> itrash) {
                    idx--;  // in wavefront obj all indices start at 1, not zero
                    f.push_back(idx);
                    uvPos.push_back(uv);
                }
                faces_.push_back(f);
                uvs_.push_back(uvPos);
            } else if (!line.compare(0, 4, "vt  ")) {
                Vec3f vt;
                float vt1, vt2, vt3;
                std::string trash;
                iss >> trash;
                while (iss >> vt1 >> vt2 >> vt3) {
                    vt.x = vt1;
                    vt.y = vt2;
                    vt.z = vt3;
                }
                uv_.push_back(vt);
            }
        }
        std::cerr << "# v# " << vertices_.size() << " f# " << faces_.size() << " vt# " << uv_.size() << std::endl;
        std::cout << "Finished reading the file" << std::endl;
        loadTexture(fileName, "_diffuse.tga", diffusemap_);
        objFile.close();
    }
}

Model::~Model() {
    if (!faces_.empty())
        faces_.pop_back();
}

int Model::nverts() {
    return vertices_.size();
}

int Model::nfaces() {
    return faces_.size();
}

Vec3f Model::vertex(int i) {
    return vertices_[i];
}

Vec2i Model::uv(int iface, int nvert) {
    int idx = faces_[iface][nvert];
    return Vec2i(uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height());
}

std::vector<int> Model::uvs(int i) {
    return uvs_[i];
}
 
std::vector<int> Model::face(int i) {
    return faces_[i];
}

void Model::loadTexture(std::string filename, const char *suffix, TGAImage &img) {
    std::string textureFile(filename);
    size_t dot = textureFile.find_last_of(".");
    size_t slash = textureFile.find_last_of("/");
    if (dot != std::string::npos) {
        textureFile = "texture/" + textureFile.substr(slash+1, dot-slash-1) + std::string(suffix);
        std::cerr << "Texture file " << textureFile << " loading " << (img.read_tga_file(textureFile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}

TGAColor Model::diffuse(Vec2i uv) {
    return diffusemap_.get(uv.x, uv.y);
}