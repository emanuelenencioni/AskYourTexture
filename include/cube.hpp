#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <glad/glad.h>
#include "mesh_utils.hpp"

struct Face {
    float color[3];
    float normal[3];
    float tangent[3];
    int tris[2][3];
    float uvs[2][3][2];
};

class Cube {
public:
    Cube();
    ~Cube();

    void setupGL();
    void draw() const;

    const PBRVertex* vertexData() const { return vertices.data(); }
    std::size_t vertexCount() const { return vertices.size(); }
    
    std::vector<float> getCorners() const;

private:
    void buildVertices();
    void tris_from_exact_position();

    Face faces[6];
    std::vector<PBRVertex> vertices;
    float corners[8][3];

    unsigned int vbo;
    unsigned int vao;
};

#endif
