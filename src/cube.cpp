#include "cube.hpp"


Cube::Cube(): VAO_(0), VBO_(0)  {
    // 8 corners, single source of truth (matches your main.cpp ordering)
    float c[8][3] = {
        {-0.6f,-0.6f,-0.6f}, { 0.6f,-0.6f,-0.6f}, {-0.6f, 0.6f,-0.6f}, { 0.6f, 0.6f,-0.6f},
        {-0.6f,-0.6f, 0.6f}, { 0.6f,-0.6f, 0.6f}, {-0.6f, 0.6f, 0.6f}, { 0.6f, 0.6f, 0.6f},
    };
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 3; ++j)
            corners_[i][j] = c[i][j];

    tris_from_exact_position();
    // color/normal are given; the corner INDICES (tris) must be wound outward.
    float colors[6][3] = {
        {1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,0,1}
    };
    float normals[6][3] = {
        {0,0,1},{0,0,-1},{-1,0,0},{1,0,0},{0,1,0},{0,-1,0}
    };
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 3; ++j) {
            faces_[i].color[j]  = colors[i][j];
            faces_[i].normal[j] = normals[i][j];
        }
    }

    buildVertices();
}

// unsigned int Cube::findNormal(const float[3] in) {
//     for(int i=0; i<6; i++){
//         for
//         if faces_[i].normal
//     }
// }


void Cube::buildVertices() {
    vertices_.clear();
    for (int f = 0; f < 6; ++f) {
        for (int t = 0; t < 2; ++t) {
            for (int k = 0; k < 3; ++k) {
                int idx = faces_[f].tris[t][k];
                for (int a = 0; a < 3; ++a) vertices_.push_back(corners_[idx][a]);
                for (int a = 0; a < 3; ++a) vertices_.push_back(faces_[f].color[a]);
                for (int a = 0; a < 3; ++a) vertices_.push_back(faces_[f].normal[a]);
            }
        }
    }
}

void Cube::setupGL() {
    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float),
                 vertices_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)24);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


Cube::~Cube() {
    if (VAO_) glDeleteVertexArrays(1, &VAO_);
    if (VBO_) glDeleteBuffers(1, &VBO_);
}

void Cube::draw() const {
    glBindVertexArray(VAO_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount());
}

void Cube::tris_from_exact_position() {
    // Face 0 (+Z, red) - Shared diagonal: 4-7
    faces_[0].tris[0][0]=4; faces_[0].tris[0][1]=5; faces_[0].tris[0][2]=7;
    faces_[0].tris[1][0]=4; faces_[0].tris[1][1]=7; faces_[0].tris[1][2]=6;

    // Face 1 (-Z, green) - Shared diagonal: 0-3
    faces_[1].tris[0][0]=0; faces_[1].tris[0][1]=2; faces_[1].tris[0][2]=3;
    faces_[1].tris[1][0]=0; faces_[1].tris[1][1]=3; faces_[1].tris[1][2]=1;

    // Face 2 (-X, blue) - Shared diagonal: 0-6
    faces_[2].tris[0][0]=0; faces_[2].tris[0][1]=4; faces_[2].tris[0][2]=6;
    faces_[2].tris[1][0]=0; faces_[2].tris[1][1]=6; faces_[2].tris[1][2]=2;

    // Face 3 (+X, yellow) - Shared diagonal: 1-7
    faces_[3].tris[0][0]=1; faces_[3].tris[0][1]=5; faces_[3].tris[0][2]=7;
    faces_[3].tris[1][0]=1; faces_[3].tris[1][1]=7; faces_[3].tris[1][2]=3;

    // Face 4 (+Y, cyan) - Shared diagonal: 2-7
    faces_[4].tris[0][0]=2; faces_[4].tris[0][1]=6; faces_[4].tris[0][2]=7;
    faces_[4].tris[1][0]=2; faces_[4].tris[1][1]=7; faces_[4].tris[1][2]=3;

    // Face 5 (-Y, magenta) - Shared diagonal: 0-5
    faces_[5].tris[0][0]=0; faces_[5].tris[0][1]=1; faces_[5].tris[0][2]=5;
    faces_[5].tris[1][0]=0; faces_[5].tris[1][1]=5; faces_[5].tris[1][2]=4;
}