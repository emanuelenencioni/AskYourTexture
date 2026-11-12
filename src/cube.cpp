#include "cube.hpp"

Cube::Cube() : vao(0), vbo(0) {
    // 8 corners, single source of truth (matches your main.cpp ordering)
    float c[8][3] = {
        {-0.6f, -0.6f, -0.6f}, // 0
        { 0.6f, -0.6f, -0.6f}, // 1
        {-0.6f,  0.6f, -0.6f}, // 2
        { 0.6f,  0.6f, -0.6f}, // 3
        {-0.6f, -0.6f,  0.6f}, // 4
        { 0.6f, -0.6f,  0.6f}, // 5
        {-0.6f,  0.6f,  0.6f}, // 6
        { 0.6f,  0.6f,  0.6f}, // 7
    };
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            corners[i][j] = c[i][j];
        }
    }

    tris_from_exact_position();

    float colors[6][3] = {
        {1.0f, 0.0f, 0.0f}, // +Z red
        {0.0f, 1.0f, 0.0f}, // -Z green
        {0.0f, 0.0f, 1.0f}, // -X blue
        {1.0f, 1.0f, 0.0f}, // +X yellow
        {0.0f, 1.0f, 1.0f}, // +Y cyan
        {1.0f, 0.0f, 1.0f}  // -Y magenta
    };

    float normals[6][3] = {
        { 0.0f,  0.0f,  1.0f}, // +Z
        { 0.0f,  0.0f, -1.0f}, // -Z
        {-1.0f,  0.0f,  0.0f}, // -X
        { 1.0f,  0.0f,  0.0f}, // +X
        { 0.0f,  1.0f,  0.0f}, // +Y
        { 0.0f, -1.0f,  0.0f}  // -Y
    };

    float tangents[6][3] = {
        { 1.0f,  0.0f,  0.0f}, // +Z face tangent is +X
        {-1.0f,  0.0f,  0.0f}, // -Z face tangent is -X
        { 0.0f,  0.0f,  1.0f}, // -X face tangent is +Z
        { 0.0f,  0.0f, -1.0f}, // +X face tangent is -Z
        { 1.0f,  0.0f,  0.0f}, // +Y face tangent is +X
        { 1.0f,  0.0f,  0.0f}  // -Y face tangent is +X
    };

    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 3; ++j) {
            faces[i].color[j]   = colors[i][j];
            faces[i].normal[j]  = normals[i][j];
            faces[i].tangent[j] = tangents[i][j];
        }
    }

    buildVertices();
}

void Cube::buildVertices() {
    vertices.clear();
    for (int f = 0; f < 6; ++f) {
        for (int t = 0; t < 2; ++t) {
            for (int k = 0; k < 3; ++k) {
                int idx = faces[f].tris[t][k];
                PBRVertex v = {
                    {corners[idx][0], corners[idx][1], corners[idx][2]},
                    {faces[f].color[0], faces[f].color[1], faces[f].color[2]},
                    {faces[f].normal[0], faces[f].normal[1], faces[f].normal[2]},
                    {faces[f].uvs[t][k][0], faces[f].uvs[t][k][1]},
                    {faces[f].tangent[0], faces[f].tangent[1], faces[f].tangent[2]}
                };
                vertices.push_back(v);
            }
        }
    }
}

void Cube::setupGL() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PBRVertex),
                 vertices.data(), GL_STATIC_DRAW);

    setupPBRAttribPointers();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Cube::~Cube() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
}

void Cube::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
    glBindVertexArray(0);
}

std::vector<float> Cube::getCorners() const {
    std::vector<float> v;
    v.reserve(8 * 3);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            v.push_back(corners[i][j]);
        }
    }
    return v;
}

void Cube::tris_from_exact_position() {
    // Face 0 (+Z, red) - corners: 4, 5, 7, 6
    faces[0].tris[0][0] = 4; faces[0].tris[0][1] = 5; faces[0].tris[0][2] = 7;
    faces[0].uvs[0][0][0] = 0.0f; faces[0].uvs[0][0][1] = 0.0f;
    faces[0].uvs[0][1][0] = 1.0f; faces[0].uvs[0][1][1] = 0.0f;
    faces[0].uvs[0][2][0] = 1.0f; faces[0].uvs[0][2][1] = 1.0f;

    faces[0].tris[1][0] = 4; faces[0].tris[1][1] = 7; faces[0].tris[1][2] = 6;
    faces[0].uvs[1][0][0] = 0.0f; faces[0].uvs[1][0][1] = 0.0f;
    faces[0].uvs[1][1][0] = 1.0f; faces[0].uvs[1][1][1] = 1.0f;
    faces[0].uvs[1][2][0] = 0.0f; faces[0].uvs[1][2][1] = 1.0f;

    // Face 1 (-Z, green) - corners: 0, 2, 3, 1
    faces[1].tris[0][0] = 0; faces[1].tris[0][1] = 2; faces[1].tris[0][2] = 3;
    faces[1].uvs[0][0][0] = 1.0f; faces[0].uvs[0][0][1] = 0.0f;
    faces[1].uvs[0][1][0] = 1.0f; faces[1].uvs[0][1][1] = 1.0f;
    faces[1].uvs[0][2][0] = 0.0f; faces[1].uvs[0][2][1] = 1.0f;

    faces[1].tris[1][0] = 0; faces[1].tris[1][1] = 3; faces[1].tris[1][2] = 1;
    faces[1].uvs[1][0][0] = 1.0f; faces[1].uvs[1][0][1] = 0.0f;
    faces[1].uvs[1][1][0] = 0.0f; faces[1].uvs[1][1][1] = 1.0f;
    faces[1].uvs[1][2][0] = 0.0f; faces[1].uvs[1][2][1] = 0.0f;

    // Face 2 (-X, blue) - corners: 0, 4, 6, 2
    faces[2].tris[0][0] = 0; faces[2].tris[0][1] = 4; faces[2].tris[0][2] = 6;
    faces[2].uvs[0][0][0] = 0.0f; faces[2].uvs[0][0][1] = 0.0f;
    faces[2].uvs[0][1][0] = 1.0f; faces[2].uvs[0][1][1] = 0.0f;
    faces[2].uvs[0][2][0] = 1.0f; faces[2].uvs[0][2][1] = 1.0f;

    faces[2].tris[1][0] = 0; faces[2].tris[1][1] = 6; faces[2].tris[1][2] = 2;
    faces[2].uvs[1][0][0] = 0.0f; faces[2].uvs[1][0][1] = 0.0f;
    faces[2].uvs[1][1][0] = 1.0f; faces[2].uvs[1][1][1] = 1.0f;
    faces[2].uvs[1][2][0] = 0.0f; faces[2].uvs[1][2][1] = 1.0f;

    // Face 3 (+X, yellow) - corners: 1, 5, 7, 3
    faces[3].tris[0][0] = 1; faces[3].tris[0][1] = 7; faces[3].tris[0][2] = 5;
    faces[3].uvs[0][0][0] = 1.0f; faces[3].uvs[0][0][1] = 0.0f;
    faces[3].uvs[0][1][0] = 0.0f; faces[3].uvs[0][1][1] = 1.0f;
    faces[3].uvs[0][2][0] = 0.0f; faces[3].uvs[0][2][1] = 0.0f;

    faces[3].tris[1][0] = 1; faces[3].tris[1][1] = 3; faces[3].tris[1][2] = 7;
    faces[3].uvs[1][0][0] = 1.0f; faces[3].uvs[1][0][1] = 0.0f;
    faces[3].uvs[1][1][0] = 1.0f; faces[3].uvs[1][1][1] = 1.0f;
    faces[3].uvs[1][2][0] = 0.0f; faces[3].uvs[1][2][1] = 1.0f;

    // Face 4 (+Y, cyan) - corners: 2, 6, 7, 3
    faces[4].tris[0][0] = 2; faces[4].tris[0][1] = 6; faces[4].tris[0][2] = 7;
    faces[4].uvs[0][0][0] = 0.0f; faces[4].uvs[0][0][1] = 0.0f;
    faces[4].uvs[0][1][0] = 0.0f; faces[4].uvs[0][1][1] = 1.0f;
    faces[4].uvs[0][2][0] = 1.0f; faces[4].uvs[0][2][1] = 1.0f;

    faces[4].tris[1][0] = 2; faces[4].tris[1][1] = 7; faces[4].tris[1][2] = 3;
    faces[4].uvs[1][0][0] = 0.0f; faces[4].uvs[1][0][1] = 0.0f;
    faces[4].uvs[1][1][0] = 1.0f; faces[4].uvs[1][1][1] = 1.0f;
    faces[4].uvs[1][2][0] = 1.0f; faces[4].uvs[1][2][1] = 0.0f;

    // Face 5 (-Y, magenta) - corners: 0, 1, 5, 4
    faces[5].tris[0][0] = 0; faces[5].tris[0][1] = 1; faces[5].tris[0][2] = 5;
    faces[5].uvs[0][0][0] = 0.0f; faces[5].uvs[0][0][1] = 0.0f;
    faces[5].uvs[0][1][0] = 1.0f; faces[5].uvs[0][1][1] = 0.0f;
    faces[5].uvs[0][2][0] = 1.0f; faces[5].uvs[0][2][1] = 1.0f;

    faces[5].tris[1][0] = 0; faces[5].tris[1][1] = 5; faces[5].tris[1][2] = 4;
    faces[5].uvs[1][0][0] = 0.0f; faces[5].uvs[1][0][1] = 0.0f;
    faces[5].uvs[1][1][0] = 1.0f; faces[5].uvs[1][1][1] = 1.0f;
    faces[5].uvs[1][2][0] = 0.0f; faces[5].uvs[1][2][1] = 1.0f;
}
