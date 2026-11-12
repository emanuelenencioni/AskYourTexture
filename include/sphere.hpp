#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <vector>
#include <glad/glad.h>
#include <math.h>
#include "mesh_utils.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Sphere {
    public:
        Sphere(float radius = 1.0f, unsigned int rings = 64, unsigned int sectors = 64);
        ~Sphere();

        void setupGL();
        void draw() const;
        unsigned int getIndexCount() const { return indexCount; }

    private:
        float radius;
        unsigned int rings;
        unsigned int sectors;

        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo; // element buffer object, store the indices in vram.
        unsigned int indexCount;

        std::vector<PBRVertex> vertices;
        std::vector<unsigned int> indices;

        void generateMesh();
};

#endif