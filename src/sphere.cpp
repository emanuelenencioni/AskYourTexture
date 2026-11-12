#include "sphere.hpp"



Sphere::Sphere(float radius, unsigned int rings, unsigned int sectors)
    : radius(radius), rings(rings), sectors(sectors) {
    generateMesh();
}

Sphere::~Sphere() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Sphere::generateMesh() {
    float v,theta, u, phi;
    float x,y,z;

    int k1, k2;

    //populate vertices
    for(int i=0;i<=rings;i++) {
        v = float(i)/float(rings);
        theta = v * M_PI;
        for(int j=0;j<=sectors; j++) {
            u = float(j)/float(sectors);
            phi = u * 2.0 * M_PI;
            x = radius * sin(theta) * cos(phi);
            y = radius * cos(theta);
            z = radius * sin(theta) * sin(phi);
            PBRVertex vertex = {
                {x, y, z},                          // pos
                {1.0f, 1.0f, 1.0f},                 // color (default white)
                {x / radius, y / radius, z / radius}, // normal (unit length!)
                {u, 1.0f - v},                      // uv
                {-sin(phi), 0.0f, cos(phi)}         // tangent
            };
            vertices.push_back(vertex);
            
            if (i < rings && j < sectors) {
                k1 = i * (sectors + 1) + j;
                k2 = k1 + sectors + 1;
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1+1);

                indices.push_back(k1+1);
                indices.push_back(k2);
                indices.push_back(k2+1);
            }
            
        }
    }

    indexCount =static_cast<unsigned int>(indices.size());
}

void Sphere::setupGL() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PBRVertex), vertices.data(),
        GL_STATIC_DRAW);

    setupPBRAttribPointers();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                    indices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void Sphere::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}