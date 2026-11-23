#include "wall.hpp"

Wall::Wall(float width, float height)
    : width(width), height(height), vao(0), vbo(0), ebo(0), indexCount(0)
{
    generateMesh();
}

Wall::~Wall() {
    if (ebo != 0) glDeleteBuffers(1, &ebo);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
}

void Wall::generateMesh() {
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    vertices.push_back(genVertex(-hw, -hh, 0.0f, 0.0f));
    vertices.push_back(genVertex(hw, -hh, 1.0f, 0.0f));
    vertices.push_back(genVertex(hw, hh, 1.0f, 1.0f));
    vertices.push_back(genVertex(-hw, hh, 0.0f, 1.0f));

    indices = {
        0 ,1, 2,
        2, 3, 0
    };
    indexCount = indices.size();
}

PBRVertex Wall::genVertex(float x, float y, float u, float v){
    PBRVertex vertex = {
        {x, y, 0.0f},
        {1.0f, 1.0f, 1.0f}, 
        {0.0f, 0.0f, 1.0f}, 
        {u, v},    // uv
        {1.0f, 0.0f, 0.0f}// tangent
    };
    return vertex;
}


void Wall::setupGL() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PBRVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    setupPBRAttribPointers();

    glBindVertexArray(0);
}

void Wall::draw() const {
    if (vao == 0 || indexCount == 0) return;

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
