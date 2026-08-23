#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <glad/glad.h>

struct Face {
    float color[3];
    float normal[3];
    int   tris[2][3];
};

class Cube {
    public:
        Cube();
        ~Cube();

        void setupGL();
        void draw() const;

        const float* vertexData() const { return vertices_.data(); }
        std::size_t vertexCount() const { return vertices_.size() / 9; }
        
        std::vector<float> getCorners() const;

    private:

        void buildVertices();
        void tris_from_exact_position();

        Face  faces_[6];
        std::vector<float> vertices_;
        float corners_[8][3];

        unsigned int VBO_;
        unsigned int VAO_;
};

#endif