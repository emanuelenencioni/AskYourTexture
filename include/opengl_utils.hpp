#ifndef OPENGL_UTILS_HPP
#define OPENGL_UTILS_HPP

#include <string>
#include <math.h>

unsigned int generateProgram(const char** vertexShader, const char** fragmentShader);
bool checkShader(unsigned int shader);
int loadTexture(const std::string& path);
void perspective(float* m, float fovRadians, float aspect, float near, float far);


#endif
