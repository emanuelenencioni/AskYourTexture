#include "opengl_utils.hpp"
#include <glad/glad.h>
#include <iostream>
#include "stb_image.h"

int loadTexture(const std::string& path) {
	int w, h, channels;
	unsigned int id;
	float maxAniso = 0.0f;

	unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);   // force RGBA

	if (!data){
		std::cout << "texture load failed: " << path << "\n";
		return -1;
	}

	glGenTextures(1, &id); 
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	// Mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //trilinear filtering

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

	// Anisotropic filtering
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso * 0.5f);
	
	stbi_image_free(data);
	return id;
}

unsigned int generateProgram(const char** vs, const char** fs) {
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader, 1, vs, NULL);
	glShaderSource(fragmentShader, 1, fs, NULL);
	glCompileShader(vertexShader);
	checkShader(vertexShader);

	glCompileShader(fragmentShader);
	checkShader(fragmentShader);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint ok;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if(!ok) {
		char log[512];
		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		std::cout << "LINK ERROR: " << log << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

bool checkShader(unsigned int shader) {
	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[512];
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		std::cout << log;
		return false;
	}
	return true;
}

void perspective(float* m, float fovRadians, float aspect, float near, float far) {
	for(int i=0; i<16; i++) m[i] = 0.0f;
	m[0] = 1/(aspect * tan(fovRadians/2));     // scale X
	m[5] = 1/tan(fovRadians/2);                // scale Y
	m[10]= (far+near)/(near-far);       // maps z
	m[11]= -1;                          // ← the "w" driver → perspective depth
	m[14]= (2*far*near)/(near-far);     // z offset
}