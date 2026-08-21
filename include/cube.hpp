#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <glad/glad.h>

class Cube {
public:
    Cube();
    ~Cube();

    void setupGL();
    void draw() const;

    const float* vertexData() const { return vertices_.data(); }
    std::size_t vertexCount() const { return vertices_.size() / 9; }
    

private:
    struct Face {
        float color[3];
        float normal[3];
        int   tris[2][3];
    };

    float corners_[8][3];
    Face  faces_[6];
    std::vector<float> vertices_;

    unsigned int VBO_;
    unsigned int VAO_;

    void buildVertices();
    void tris_from_exact_position();
};

#endif