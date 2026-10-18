#ifndef IBL_UTILS_H
#define IBL_UTILS_H

const float eyeIBL[3] = {0.0f, 0.0f, 0.0f};

// The 6 directions and their OpenGL cubemap up-vectors:
const float targetsIBL[6][3] = {
    { 1.0f,  0.0f,  0.0f}, // +X
    {-1.0f,  0.0f,  0.0f}, // -X
    { 0.0f,  1.0f,  0.0f}, // +Y
    { 0.0f, -1.0f,  0.0f}, // -Y
    { 0.0f,  0.0f,  1.0f}, // +Z
    { 0.0f,  0.0f, -1.0f}  // -Z
};

const float upsIBL[6][3] = {
    {0.0f, -1.0f,  0.0f}, // +X
    {0.0f, -1.0f,  0.0f}, // -X
    {0.0f,  0.0f,  1.0f}, // +Y
    {0.0f,  0.0f, -1.0f}, // -Y
    {0.0f, -1.0f,  0.0f}, // +Z
    {0.0f, -1.0f,  0.0f}  // -Z
};

#endif