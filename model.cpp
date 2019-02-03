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
        std::cout << "File is opened!" << std::endl;
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
                int itrash, idx;
                iss >> trash;
                while (iss >> idx >> trash >> itrash >> trash >> itrash)
                {
                    idx--; // in wavefront obj all indices start at 1, not zero
                    f.push_back(idx);
                }
                // std::cout << "f is ";
                // for(int i = 0; i < 3; i++)
                //     std::cout << f[i] << " ";
                // std::cout << std::endl;
                faces_.push_back(f);
            }
        }
        std::cerr << "# v# " << vertices_.size() << " f# " << faces_.size() << std::endl;
        std::cout << "Finished reading the file" << std::endl;
        objFile.close();
    }
}

Model::~Model()
{
    std::cout << "I am failing here" << std::endl;
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

std::vector<int> Model::face(int i) {
    return faces_[i];
}