
// ----- BASIC SHADERS -----

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // posizione in ingresso letteralmente "in"
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 transform; // for the camera rotation

out vec3 ourColor;
out vec3 worldPos;
out vec3 worldNormal; 
void main()
{
    gl_Position = transform * vec4(aPos, 1.0f);   // coord. omogenee.
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

// ----- TEXTURE IMPLEMENTATION -----

const char* vertexShaderTexture = R"(
#version 330 core

layout (location = 0) in vec3 aPos; // posizione in ingresso letteralmente "in"
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

layout (location = 3) in vec2 aTextCoord;

uniform mat4 transform; // for the camera rotation

out vec3 ourColor;
out vec3 worldPos;
out vec3 worldNormal; 

out vec2 textCoord;
void main()
{
    gl_Position = transform * vec4(aPos, 1.0f);   // coord. omogenee.
    ourColor = aColor;
    worldPos = aPos;
    worldNormal = aNormal;
    textCoord = aTextCoord;
}
)";

const char* fragmentShaderTexture = R"(
#version 330 core

uniform sampler2D normalTexture;

in vec3 ourColor;
in vec3 worldPos;
in vec3 worldNormal;
in vec2 textCoord;

uniform vec3 lightPos;
uniform sampler2D ourTexture;


out vec4 FragColor; // vettore di output = colore
void main() {
    //vec3 n = normalize(worldNormal); //old normals.
    vec3 l = normalize(lightPos - worldPos);
    vec2 tiledUV = textCoord;

    vec3 n_t = texture(normalTexture, tiledUV).xyz * 2.0 - 1.0; // RAW no sRGB decode.
    const vec3 T = vec3(1.0, 0.0, 0.0);
    const vec3 B = vec3(0.0, 0.0, 1.0);
    const vec3 N = vec3(0.0, 1.0, 0.0);
    vec3 n  = normalize (mat3(T,B,N)*n_t);

    float brightness = 0.3 + 0.7*max(dot(n, l), 0.0);

    // old code
    vec3 baseColor = pow(texture(ourTexture, textCoord).rgb, vec3(2.2));
    FragColor = vec4(baseColor * brightness, 1.0);  

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

void main() {
    vec3 hdr = texture(hdrBuffer, uv).rgb;
    vec3 toneMapped = vec3(1.0) -exp(-hdr*1.0);
    FragColor =  vec4(pow(toneMapped, vec3(mix(1.0, 1.0/2.2, uGamma))), 1.0);
})";
