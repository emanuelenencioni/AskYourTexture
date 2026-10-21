#ifndef IBL_HPP
#define IBL_HPP

#include <glad/glad.h>
#include <string>
#include <iostream>

#include "cube.hpp" 
#include "shaders.hpp"
#include "math_utils.hpp"
#include "stb_image.h"
#include "opengl_utils.hpp"

class Cube; // forward declaration for ref only.

class IBLSystem {
public:
    explicit IBLSystem(const std::string& hdrPath);
    ~IBLSystem();

    void bake(const Cube& cube);

    // Getters 
    unsigned int getEnvCubemap() const { return envCubemap; }
    unsigned int getIrradianceMap() const { return irradianceMap; }
    int loadHDR(const std::string& path);

private:
    std::string hdrPath;
    
    int hdrTexture;

    // output of bakes
    unsigned int envCubemap;
    unsigned int irradianceMap;

    // Temporal resources for off-screen rendering
    unsigned int captureFBO;
    unsigned int captureRBO;

    // internal shader programs
    unsigned int equiProgram;
    unsigned int irradianceProgram;

    // internal helpers
    void setupFramebuffers();
    void bakeEquirectangular(const Cube& cube);
    void bakeIrradiance(const Cube& cube);
    void bake(const Cube& cube, unsigned int &cubemap, const unsigned int cube_size, const unsigned int program);


};

extern const char* fragmentEquirectangular;
extern const char* vertexEquirectangular;
extern const char* fragmentIrradiance;

#endif