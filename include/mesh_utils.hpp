#ifndef MESH_UTILS_HPP
#define MESH_UTILS_HPP

#include <glad/glad.h>

struct PBRVertex {
    float pos[3];       // location 0 (offset 0)
    float color[3];     // location 1 (offset 12)
    float normal[3];    // location 2 (offset 24)
    float uv[2];        // location 3 (offset 36)
    float tangent[3];   // location 4 (offset 44)
};

inline void setupPBRAttribPointers() {
    GLsizei stride = sizeof(PBRVertex); // 14 * sizeof(float) = 56 bytes

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
}

#endif
