#include "ibl.hpp"
#include "opengl_utils.hpp"
#include "stb_image.h"
#include <iostream>

namespace {
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
} // anonymous namespace

IBLSystem::IBLSystem(const std::string& path): hdrPath(path) {
    hdrTexture = loadHDR(hdrPath);
    irradianceProgram = generateProgram(&vertexEquirectangular, &fragmentIrradiance);
    equiProgram = generateProgram(&vertexEquirectangular, &fragmentEquirectangular);
    
    // setupFramebuffers
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
}
IBLSystem::~IBLSystem() {
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);
}

void IBLSystem::bake(const Cube& cube) {
    bakeEquirectangular(cube);
    bakeIrradiance(cube);
}

void IBLSystem::bakeEquirectangular(const Cube& cube) {
    glUseProgram(equiProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glUniform1i(glGetUniformLocation(equiProgram, "equirectangularMap"), 0);

    bake(cube, envCubemap, 512, equiProgram, 1);
}

// params: cube_size, program
void IBLSystem::bakeIrradiance(const Cube& cube) {
    glUseProgram(irradianceProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glUniform1i(glGetUniformLocation(irradianceProgram, "environmentMap"), 1);

    bake(cube, irradianceMap, 32, irradianceProgram, 2);
}
/**
 * @brief 
 * 
 * @param cube 
 * @param cubemap 
 * @param cube_size 
 * @param program 
 * @param destIdx texture index of the source texture
 */
void IBLSystem::bake(const Cube& cube, unsigned int &cubemap, const unsigned int cube_size, const unsigned int program,  const unsigned int srcIdx) {
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cube_size, cube_size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glActiveTexture(GL_TEXTURE0 + srcIdx);
    glGenTextures(srcIdx, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	// allocating each face as 2D image
	for(int i=0;i<6;i++){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cube_size, cube_size, 0,  GL_RGB, GL_FLOAT, NULL);
	}
	// texture sampling configs -> first 3 avoid to do warp-aroud, the latest 2  set bilinear
	//  interpolation for magnification and minification (texel appear larger or smaller on screen)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glViewport(0,0,cube_size,cube_size);
	glUseProgram(program);
	
	float captureProjection[16];
	perspective(captureProjection, M_PI / 2.0f, 1.0f, 0.1f, 10.0f);
	float captureViews[6][16];

	GLint ProjLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, captureProjection);
	
	GLint ViewLoc = glGetUniformLocation(program, "view");
	// "Bake" loop
	for(int i=0;i<6;i++){
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		lookAt(eyeIBL, targetsIBL[i], upsIBL[i], captureViews[i]); // IBL variables can be found in ibl_utils.hpp
		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, captureViews[i]);	
		cube.draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



int IBLSystem::loadHDR(const std::string& path) {
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);

	if (!data) {
		std::cout << "Failed to load HDR image: " << path << std::endl;
		return -1;
	}

	unsigned int hdrTexture;
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);   // ← NOT REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);   //   (fullscreen quad touches 0 and 1 exactly)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	stbi_image_free(data);
	return hdrTexture;
}




const char* fragmentEquirectangular = R"(
        #version 330 core
        out vec4 FragColor;
        in vec3 localPos;

        uniform sampler2D equirectangularMap;

        const vec2 invAtan = vec2(0.1591549, 0.3183098); // 1/(2*PI), 1/PI

        vec2 SampleSphericalMap(vec3 v) {
            // TODO 1: compute the horizontal angle (atan) from v.z and v.x
            // TODO 2: compute the vertical angle (asin) from v.y
            // TODO 3: scale by invAtan and offset by 0.5 to land in [0.0, 1.0] range

            vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
            uv = uv * invAtan + 0.5;
            return uv;
        }

        void main() {
            // Normalize the ray direction:
            vec3 v = normalize(localPos);

            // Sample using the spherical UV coordinates:
            vec2 uv = SampleSphericalMap(v);
            vec3 color = texture(equirectangularMap, uv).rgb;

            FragColor = vec4(color, 1.0);
        }
)";

const char* vertexEquirectangular = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    out vec3 localPos;

    uniform mat4 projection;
    uniform mat4 view;

    void main() {
        localPos = aPos;
        // Standard projection * view transformation:
        gl_Position = projection * view * vec4(localPos, 1.0);
    }
)";


const char* fragmentIrradiance = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 localPos;

    uniform samplerCube environmentMap;

    const float PI = 3.14159265359;

    // Convolution of the hemisphere around normal N
    vec3 convolve(vec3 N) {
        vec3 irradiance = vec3(0.0);

        // Tangent space basis around N:
        vec3 up    = vec3(0.0, 1.0, 0.0);
        vec3 right = normalize(cross(up, N));
        up         = normalize(cross(N, right));

        float sampleDelta = 0.025;
        float nrSamples = 0.0;

        for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
            for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {

                // Spherical to tangent-space direction:
                vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

                // Tangent space to world space:
                vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

                // Sample sharp environment and weight by Lambert + solid angle:
                irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
                nrSamples++;
            }
        }
        return PI * irradiance * (1.0 / nrSamples);
    }

    void main() {
        vec3 N = normalize(localPos);
        FragColor = vec4(convolve(N), 1.0);
    }
)";