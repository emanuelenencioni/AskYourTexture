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
    static constexpr unsigned int PREFILTER_MIP_LEVELS = 5;
    // Getters 
    unsigned int getEnvCubemap() const { return envCubemap; }
    unsigned int getIrradianceMap() const { return irradianceMap; }
    unsigned int getPrefilterMap() const { return prefilterMap; }
    unsigned int getBRDFLUT() const { return brdfLUTTexture; }
    int loadHDR(const std::string& path);

private:
    std::string hdrPath;
    
    int hdrTexture;

    // output of bakes
    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUTTexture;

    // Temporal resources for off-screen rendering
    unsigned int captureFBO;
    unsigned int captureRBO;

    unsigned int quadVAO;
    unsigned int quadVBO;

    // internal shader programs
    unsigned int equiProgram;
    unsigned int irradianceProgram;
    unsigned int prefilterProgram;
    unsigned int brdfProgram;

    // internal helpers
    void allocateCubemap(unsigned int &cubemap, const unsigned int size, unsigned int internalFormat = GL_RGB32F, unsigned int miplevels = 1, unsigned int minfilter = GL_LINEAR);
    void captureFaces(const Cube& cube, unsigned int cubemap, unsigned int size, unsigned int program, unsigned int level = 0);
    void bakeEquirectangular(const Cube& cube);
    void bakeIrradiance(const Cube& cube);
    void bakePrefilter(const Cube& cube);
    void bakeBRDF();    

    void setupQuad();
    void renderQuad();

};

extern const char* fragmentEquirectangular;
extern const char* vertexEquirectangular;
extern const char* fragmentIrradiance;
extern const char* fragmentPrefilter;
extern const char* fragmentBRDF;

#endif