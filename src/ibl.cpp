#include "ibl.hpp"


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
    prefilterProgram = generateProgram(&vertexEquirectangular, &fragmentPrefilter);
    brdfProgram = generateProgram(&vertexShaderQuad, &fragmentBRDF);
    
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
    bakePrefilter(cube);
    bakeBRDF();
}

void IBLSystem::bakeEquirectangular(const Cube& cube) {
    glUseProgram(equiProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glUniform1i(glGetUniformLocation(equiProgram, "equirectangularMap"), 0);

    allocateCubemap(envCubemap,512);
    captureFaces(cube, envCubemap, 512, equiProgram);
}

// params: cube_size, program
void IBLSystem::bakeIrradiance(const Cube& cube) {
    glUseProgram(irradianceProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glUniform1i(glGetUniformLocation(irradianceProgram, "environmentMap"), 1);

    allocateCubemap(irradianceMap,32);
    captureFaces(cube, irradianceMap, 32, irradianceProgram);
}

void IBLSystem::bakePrefilter(const Cube& cube) {
    glUseProgram(prefilterProgram);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glUniform1i(glGetUniformLocation(prefilterProgram, "environmentMap"), 1);

    allocateCubemap(prefilterMap, 128, GL_RGB16F, PREFILTER_MIP_LEVELS, GL_LINEAR_MIPMAP_LINEAR);
    unsigned int roughnessLoc = glGetUniformLocation(prefilterProgram, "roughness");
    unsigned int nSampleLoc = glGetUniformLocation(prefilterProgram, "n_samples");

    for(int mip=0; mip<PREFILTER_MIP_LEVELS; ++mip){
        int mipSize = 128 >> mip;
        glUniform1f(roughnessLoc, float(mip) / float(PREFILTER_MIP_LEVELS -1)); // loading calculated roughness
        glUniform1ui(nSampleLoc, 1024u);
        captureFaces(cube, prefilterMap, mipSize, prefilterProgram, mip);
    }

}


void IBLSystem::allocateCubemap(unsigned int &cubemap, const unsigned int size, unsigned int internalFormat, unsigned int mipLevels, unsigned int minfilter) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

    for(int i=0;i<6;i++){
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, size, size, 0,  GL_RGB, GL_FLOAT, NULL);
	}
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minfilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(mipLevels > 1) {
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, mipLevels-1);
    }

}

void IBLSystem::captureFaces(const Cube& cube, unsigned int cubemap, unsigned int size, unsigned int program, unsigned int level) {
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    
    glViewport(0,0,size,size);
	glUseProgram(program);
	
	float captureProjection[16];
	perspective(captureProjection, M_PI / 2.0f, 1.0f, 0.1f, 10.0f);
	float captureViews[6][16];

	GLint ProjLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, captureProjection);
	
	GLint ViewLoc = glGetUniformLocation(program, "view");
	// "Bake" loop
	for(int i=0;i<6;i++){
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, level);
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

void IBLSystem::setupQuad() {
        float quadVertices[] = {
            // positions   // texture coords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void IBLSystem::renderQuad() {
        if (quadVAO == 0) {
            setupQuad();
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
}

void IBLSystem::bakeBRDF() {
        // 1. Allocate 512x512 2D texture with format GL_RG16F
        glGenTextures(1, &brdfLUTTexture);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 2. Attach texture to FBO and render the quad
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

        glViewport(0, 0, 512, 512);
        glUseProgram(brdfProgram);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

            color = clamp(color, vec3(0.0), vec3(500.0));

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
        vec3 up    = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
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



const char* fragmentPrefilter = R"(
    #version 330 core

    const float PI = 3.14159265359;

    float RadicalInverse_VdC(uint bits) {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    vec2 Hammersley(uint i, uint N) {
        return vec2(float(i) / float(N), RadicalInverse_VdC(i));
    }
    
    vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
        float a = roughness * roughness;         // According to Disney/Epic papers, squaring the perceptual roughness = more linear visual response
        float a2 = a * a;


        float phi = 2.0 * PI * Xi.x;  // spherical angle mapping of  Xi(2D low-discrepancy point)


        // the inverse CDF of the GGX distribution
        float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // spherical to tangent-space direction, H is the half vector.
        vec3 H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;

        // NaN check
        vec3 up    = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
        vec3 tangent   = normalize(cross(up, N));
        vec3 bitangent = cross(N, tangent);

        //  from tangent space to world space
        vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return normalize(sampleVec);
    }


    uniform uint n_samples; 
    uniform float roughness;
    uniform samplerCube environmentMap;    

    in vec3 localPos;

    out vec4 FragColor;


    void main() {
        vec3 N = normalize(localPos);
        vec3 R = N;
        vec3 V = R;

        vec3 prefilteredColor = vec3(0.0);
        float totalWeight = 0.0;
        
        for  (uint i=0u;i<n_samples;i++) {
            vec2 Xi = Hammersley(i, n_samples);
            vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            vec3 L = normalize(2.0 * dot(V,H) * H - V); // incoming light dir

            float NdotL = dot(N, L);
            if(NdotL > 0.0) {
                prefilteredColor += texture(environmentMap, L).rgb * NdotL;
                totalWeight += NdotL;
            }
        }
        FragColor = vec4(prefilteredColor / totalWeight, 1.0);
    }
)";

const char* fragmentBRDF = R"(
    #version 330 core

    const float PI = 3.14159265359;
    const uint SAMPLE_COUNT = 1024u;

    float RadicalInverse_VdC(uint bits) {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return float(bits) * 2.3283064365386963e-10; // / 0x100000000
    }

    vec2 Hammersley(uint i, uint N) {
        return vec2(float(i) / float(N), RadicalInverse_VdC(i));
    }
    
    vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
        float a = roughness * roughness;         // According to Disney/Epic papers, squaring the perceptual roughness = more linear visual response
        float a2 = a * a;


        float phi = 2.0 * PI * Xi.x;  // spherical angle mapping of  Xi(2D low-discrepancy point)


        // the inverse CDF of the GGX distribution
        float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a2 - 1.0) * Xi.y));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        // spherical to tangent-space direction, H is the half vector.
        vec3 H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;

        // NaN check
        vec3 up    = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
        vec3 tangent   = normalize(cross(up, N));
        vec3 bitangent = cross(N, tangent);

        //  from tangent space to world space
        vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return normalize(sampleVec);
    }

    float GeometrySchlickGGX(float NdotV, float roughness) {
        float a = roughness;
        float k = (a * a) / 2.0;

        float nom   = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / denom;
    }

    float GeometrySmith_IBL(float NdotV, float NdotL, float roughness) {
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    vec2 IntegrateBRDF(float NdotV, float roughness) {
        float sinTheta = sqrt(1.0 - NdotV * NdotV);
        vec3 V = vec3(sinTheta, 0.0, NdotV);
        vec3 N = vec3(0.0, 0.0, 1.0);

        // accumulators
        float A = 0.0;        
        float B = 0.0;


        for(uint i=0u;i<SAMPLE_COUNT; i++) {
            vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            vec3 L = normalize(2.0 * dot(V, H) * H - V);
            float NdotL = max(L.z, 0.0); // given N being (0,0,1)
            float NdotH = max(H.z, 0.0);
            float VdotH = max(dot(V, H), 0.0);

            if (NdotL > 0.0) {
                float G = GeometrySmith_IBL(NdotV, NdotL, roughness);
                float Gvis = (G * VdotH) / (NdotH * NdotV);
                float Fc = pow(1.0 - VdotH, 5.0); //fresnel factor (Schlick)

                A += (1.0 - Fc) * Gvis;
                B += Fc*Gvis;
            }
        }

        return vec2(A/float(SAMPLE_COUNT), B/float(SAMPLE_COUNT));
    }


    in vec2 uv;
    out vec2 FragColor;

    void main() {
        FragColor = IntegrateBRDF(uv.x, uv.y);
    }
)";