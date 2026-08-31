
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

out vec4 FragColor; // vettore di output = colore
void main() {
    vec3 n = normalize(worldNormal);
    vec3 l = normalize(lightPos - worldPos);
    float brightness = 0.3 + 0.7*max(dot(n, l), 0.0);
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

in vec3 ourColor;
in vec3 worldPos;
in vec3 worldNormal;
in vec2 textCoord;

uniform vec3 lightPos;
uniform sampler2D ourTexture;

out vec4 FragColor; // vettore di output = colore
void main() {
    vec3 n = normalize(worldNormal);
    vec3 l = normalize(lightPos - worldPos);
    float brightness = 0.3 + 0.7*max(dot(n, l), 0.0);
    vec3 baseColor = texture(ourTexture, textCoord).rgb;
    FragColor = vec4(baseColor * brightness, 1.0);  
})";

