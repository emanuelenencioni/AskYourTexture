
// ----- BASIC SHADERS -----

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // posizione in ingresso letteralmente "in"
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 viewProjection; // for the camera rotation
uniform mat4 model;

out vec3 ourColor;
out vec3 worldPos;
out vec3 worldNormal; 
void main()
{
    gl_Position = viewProjection * model * vec4(aPos, 1.0f);   // coord. omogenee.
    ourColor = aColor;
    worldPos = aPos;
    worldNormal = aNormal;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in vec3 ourColor;
in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 lightPos;

uniform float uGamma;


out vec4 FragColor; // vettore di output = colore
void main() {
    vec3 n = normalize(worldNormal);
    vec3 l = normalize(lightPos - worldPos);
    float brightness = 0.3 + 0.7*max(dot(n, l), 0.0);
    //gamma correction
    //FragColor = vec4(pow(brightness * ourColor,vec3(mix(1.0, 1.0/2.2, uGamma))), 1.0);
    
    // old code
    FragColor = vec4(ourColor * brightness, 1.0);  
    
})";

// vertex: pure passthrough — no matrix, no normals. The quad IS in final coordinates.
const char* vertexShaderQuad = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
out vec2 uv;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    uv = aUV;
})";

// fragment: test rig — sample an existing texture, tone mapping comes at step 4/5
const char* fragmentShaderQuad = R"(
#version 330 core
in vec2 uv;
uniform sampler2D hdrBuffer;   // for now: floor color texture as test content
out vec4 FragColor;

uniform float uGamma;
uniform float uExposure;

void main() {
    vec3 hdr = texture(hdrBuffer, uv).rgb;
    vec3 toneMapped = vec3(1.0) -exp(-hdr*uExposure);
    FragColor =  vec4(pow(toneMapped, vec3(mix(1.0, 1.0/2.2, uGamma))), 1.0);
})";



// PBR shaders

// VS: same shape as vertexShaderTexture (TBN pipeline from M0 + model/viewProjection)
const char* vertexShaderPBR = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 4) in vec3 aTangent;
layout (location = 3) in vec2 aTextCoord;

uniform mat4 viewProjection;
uniform mat4 model;

out vec3 ourColor;
out vec3 worldPos;
out vec3 worldNormal;
out vec3 worldTangent;
out vec3 worldBitangent;
out vec2 textCoord;
void main()
{
    gl_Position = viewProjection * model * vec4(aPos, 1.0f);
    ourColor = aColor;
    worldPos = aPos;
    worldNormal = aNormal;
    textCoord = aTextCoord;
    worldTangent = aTangent;
    worldBitangent = normalize(cross(aTangent, aNormal));
}
)";


const char* fragmentShaderPBR = R"(
#version 330 core

const float PI = 3.14159265359;
const float DIELECTRIC_F0 = 0.04;

in vec3 worldPos;
in vec3 worldNormal;
in vec3 worldTangent;
in vec3 worldBitangent;
in vec2 textCoord;

uniform samplerCube irradianceMap;
uniform sampler2D albedoTexture;   
uniform sampler2D normalTexture;    
uniform sampler2D roughnessTexture; 
uniform sampler2D aoTexture;       
uniform vec3 lightPos;
uniform vec3 viewPos;              
uniform float uMetallic;         
out vec4 FragColor;


vec3 f0_metallicMix(vec3 albedo, float metallic) {
    return vec3(DIELECTRIC_F0) + metallic * (albedo - vec3(DIELECTRIC_F0));
}

vec3 f_schlick(float VdotH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

float d_ggx(float NdotH, float roughness) {
    float alpha = pow(roughness, 2.0);
    float a2 = alpha * alpha;
    return a2 / (PI * pow(NdotH*NdotH * (a2 - 1.0) + 1.0, 2.0));
}

float k_direct(float roughness) {
    return pow(roughness + 1.0, 2.0) / 8.0;
}

float g_smith(float NdotV, float NdotL, float roughness) {
    float k = k_direct(roughness);
    float G1_v = NdotV / (NdotV * (1.0 - k) + k);
    float G1_l = NdotL / (NdotL * (1.0 - k) + k);
    return G1_v * G1_l;
}

// ----- per-pixel assembly: TODO (your port) -----

void main() {
    vec3 albedo = pow(texture(albedoTexture, textCoord).rgb, vec3(2.2));
    float roughness = texture(roughnessTexture, textCoord).r;
    float ao = texture(aoTexture, textCoord).r;
    float metallic = uMetallic;

    vec2 tiledUV = textCoord;
    vec3 n_t = texture(normalTexture, tiledUV).xyz * 2.0 - 1.0; 
    vec3 n  = normalize (mat3(worldTangent,worldBitangent,worldNormal)*n_t);

    vec3 l = normalize(lightPos - worldPos);
    vec3 v = normalize(viewPos  - worldPos);      // ← the only genuinely new vector
    vec3 h = normalize(l + v);

    float NdotV = max(dot(n,v), 1e-4);
    float NdotH = max(dot(n,h), 1e-4);
    float NdotL = max(dot(n,l), 1e-4);

    vec3 F0 = f0_metallicMix(albedo, metallic);
    vec3 F = f_schlick(dot(v,h), F0);
    float   D  = d_ggx(NdotH, roughness);
    float  G  = g_smith(NdotV, NdotL, roughness);

    vec3 radiance = vec3(1.0); // (white sun; scale later if you want)
    vec3 specular = D * G * F / (4 * NdotV * NdotL);
    vec3 kD = (1 - F) * (1 - metallic);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    vec3 irradiance = texture(irradianceMap, n).rgb;
    vec3 diffuse = irradiance*albedo;
    vec3 ambient = (kD * diffuse) * ao * 0.2;

    Lo *= ao; 
    vec3 color = ambient + Lo;
    FragColor = vec4(color, 1.0);
    }
)";


// ----- SKYBOX SHADERS -----

    const char* vertexSkybox = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;

    out vec3 TexCoords;

    uniform mat4 projection;
    uniform mat4 view;

    void main() {
        TexCoords = aPos;

        // Strip camera translation (keep rotation only so skybox stays at infinity):
        mat4 rotView = mat4(mat3(view));
        vec4 clipPos = projection * rotView * vec4(aPos, 1.0);

        // Set z to equal w (xyww) so that after perspective division (z/w),
        // the depth is exactly 1.0, cant get close to it.
        gl_Position = clipPos.xyww;
    }
    )";

    const char* fragmentSkybox = R"(
    #version 330 core
    out vec4 FragColor;
    in vec3 TexCoords;

    uniform samplerCube skybox;

    void main() {
        // TODO 2: Sample the cubemap using TexCoords (3D direction vector)
        vec3 envColor = textureLod(skybox, TexCoords, 0.0).rgb;

        FragColor = vec4(envColor, 1.0);
    }
    )";