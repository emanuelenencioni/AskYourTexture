#include "opengl_utils.hpp"
#include <glad/glad.h>
#include <iostream>

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