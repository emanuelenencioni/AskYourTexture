#ifndef WALL_HPP
#define WALL_HPP

#include <vector>
#include <glad/glad.h>

#include "mesh_utils.hpp"

class Wall {
public:
    Wall(float width = 2.0f, float height = 2.0f);
    ~Wall();

    void setupGL();
    void draw() const;

private:
    float width;
    float height;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    unsigned int indexCount;

    std::vector<PBRVertex> vertices;
    std::vector<unsigned int> indices;

    void generateMesh();
    PBRVertex genVertex(float x, float y, float u, float v);
};

#endif
