#include <glad/glad.h> //glad has some deps on GLFW so ALWAYS BEFORE
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <thread>
#include <chrono>

#include "cube.hpp"
#include "shadow.hpp"
#include "stb_image.h"
#include "shaders.hpp"
#include "math_utils.hpp"
#include "camera.hpp"


#define ROTATING_SUN 1
#define ROTATING_CAM 0


void perspective(float* m, float fovRadians, float aspect, float near, float far);
//just an helper
void mult4x4(float* out, const float* a, const float* b);

int loadTexture(const std::string path);
unsigned int generateProgram(const char** vertexShader,const char** fragmentShader);

bool checkShader(unsigned int shader);

int main(){
	float window_x_size = 1280;
	float window_y_size = 720;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	

	GLFWwindow* window = glfwCreateWindow(window_x_size,window_y_size, "AskYourTexture", NULL, NULL);
	if (window == NULL){
		std::cout<<"Failed creating window";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout<<"error 2";
		return -1;
		
	}
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// // creazione shaders
	// unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// // passo il prog.
	// glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	// glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	// // compilazione shaders
	// glCompileShader(vertexShader);
	// glCompileShader(fragmentShader);
	// unsigned int shaderProgram = glCreateProgram();
	// glAttachShader(shaderProgram, vertexShader);
	// glAttachShader(shaderProgram, fragmentShader);
	// glLinkProgram(shaderProgram);
	// //a quanto pare, una volta messi nel programma, gli shader non servono più (singoli)
	// glDeleteShader(vertexShader);
	// glDeleteShader(fragmentShader);

	unsigned int shaderProgram = generateProgram(&vertexShaderSource, &fragmentShaderSource);
	unsigned int textureProgram = generateProgram(&vertexShaderTexture, &fragmentShaderTexture);


// ----- STUFF IN THE SCENE -----

	// CUBE now has a class
	Cube cube;
	cube.setupGL();


	float floorVertices[] = { // FLOOR
		// normal (0,1,0) — gray, last two are the UV -> mapping 2D texture to 3D floor.
		-4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  0,0,
		4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  1,0,
		4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  1,1,  
		-4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  0,0,
		-4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  0,1,
		4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,  1,1,
	};
	unsigned int VBOFloor;
	glGenBuffers(1, &VBOFloor);
	glBindBuffer(GL_ARRAY_BUFFER, VBOFloor);

	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	
	unsigned int VAOFloor;
	glGenVertexArrays(1, &VAOFloor);
	glBindVertexArray(VAOFloor); 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)0); //mapping coords
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)12); //mapping color
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)24); //mapping normals
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)36); // mapping UV
	glEnableVertexAttribArray(3);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);   


	glViewport(0,0,window_x_size,window_y_size);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	glEnable(GL_DEPTH_TEST);
	
	float aspect_ratio = window_x_size / window_y_size;
	//rotation data
	float m[16];
	float fov = M_PI/3;
	float elev = 10*M_PI/180;//M_PI/6;
	float transform[16];
	float view[16] = {0}; view[0]=1; view[5]=1; view[10]=1; view[15]=1;
	float projection[16];
	perspective(projection,fov,aspect_ratio,0.1, 100);
	float R = 8.0f;
	float target[3] = {0};
	float s[3];
	GLint loc = glGetUniformLocation(shaderProgram, "transform");
	GLint texLoc = glGetUniformLocation(textureProgram, "transform");
	GLint texSamplerLoc = glGetUniformLocation(textureProgram, "ourTexture");
	GLint normalSamplerLoc = glGetUniformLocation(textureProgram, "normalTexture");


	// light source!
	float lightPos[3] = {5.0f, 5.0f, 5.0f};
	float R_light = 5.0f;
	float speed = 0.3;
	GLint lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
	GLint texLightLoc = glGetUniformLocation(textureProgram, "lightPos");

	// shadows!
	Shadow shad(cube.getCorners(), lightPos, floorVertices[1]);

	shad.setupGL();

	// textures!
	std::string imagePath = "../Ground108_4K-PNG/Ground108_4K-PNG_Color.png"; // relative to the executable
	std::string normalPath = "../Ground108_4K-PNG/Ground108_4K-PNG_NormalGL.png";
	int idTexture = loadTexture(imagePath);
	int idNormal = loadTexture(normalPath);

	//camera!
	float position[3] = {7.88,1.39,0};

	float front[3];
	for(int i=0; i<3; i++) front[i] = -position[i];
	normalize(front);

	float pitch = asin(front[1])*180/M_PI;  //input in deg.
	float yaw = atan2(front[2], front[0])*180/M_PI; 

	Camera cam(position, yaw, pitch);
	

	// Gamma toggling
	bool gammaOn = true;          // start corrected; pressing G dims it — or reverse, your call
	bool gWasPressed = false;
	GLint gammaLoc     = glGetUniformLocation(shaderProgram,  "uGamma");
	GLint texGammaLoc  = glGetUniformLocation(textureProgram, "uGamma");


	double mouseX,mouseY;

	float t, dt ,lastT;

	lastT = glfwGetTime();
	// loop
	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		t = glfwGetTime();
		dt = t - lastT;
		lastT = t;

		bool gPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
		if(gPressed && !gWasPressed){
			gammaOn = !gammaOn;
		}
		gWasPressed = gPressed;

	//  ----- ORBITING VIEW CALCULATION -----
		
		//camera orbiting
		glfwGetCursorPos(window, &mouseX, &mouseY);
		cam.processMouse(mouseX, mouseY);

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cam.processKeyboard(GLFW_KEY_W, dt);

		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cam.processKeyboard(GLFW_KEY_A, dt);

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cam.processKeyboard(GLFW_KEY_S, dt);

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cam.processKeyboard(GLFW_KEY_D, dt);


		cam.getViewMatrix(view);


	// ----- ORBITING SUN CALCULATION -----
		if(ROTATING_SUN){
			lightPos[0] = R_light * cos(speed * t);
			lightPos[1] = R_light * sin(speed*t);
		}
		
		mult4x4(transform, projection, view);
	
		glUniformMatrix4fv(loc, 1, GL_FALSE, transform);
		glUniform3fv(lightLoc, 1, lightPos); 	

		shad.update(lightPos);
		//draw floor
		glUseProgram(textureProgram);
		glUniform1i(texSamplerLoc, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glUniformMatrix4fv(texLoc, 1, GL_FALSE, transform);
		glUniform3fv(texLightLoc, 1, lightPos); 	

		// normal mapping
		glUniform1i(normalSamplerLoc, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, idNormal);


		glBindVertexArray(VAOFloor);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUniform1f(texGammaLoc, gammaOn ? 1.0f : 0.0f);   // for textureProgram users (floor)


		glUseProgram(shaderProgram);
		shad.draw();
		cube.draw();
		
		glUniform1f(gammaLoc, gammaOn ? 1.0f : 0.0f);      // for shaderProgram users (cube/shadow)


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}



void perspective(float* m, float fovRadians, float aspect, float near, float far) {
	for(int i=0; i<16; i++) m[i] = 0.0f;
	m[0] = 1/(aspect * tan(fovRadians/2));     // scale X
	m[5] = 1/tan(fovRadians/2);                // scale Y
	m[10]= (far+near)/(near-far);       // maps z
	m[11]= -1;                          // ← the "w" driver → perspective depth
	m[14]= (2*far*near)/(near-far);     // z offset
}

void mult4x4(float* out, const float* a, const float* b) {

	for (int row=0; row<4; row++){
		for(int col=0; col<4; col++){
			out[row+4*col] = 0;
			for(int k=0; k<4; k++){
				out[row+4*col] += a[k*4+row] * b[col*4+k];
			}
		}
	}
}



int loadTexture(const std::string path) {
	int w, h, channels;
	uint id;
	if(path == NULL)
		return -1;
	
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);   // force RGBA

    glGenTextures(1, &id); 
	glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
	
	stbi_image_free(data);
    return id;
}

unsigned int generateProgram(const char** vs,const char** fs) {
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShader, 1, vs, NULL);
	glShaderSource(fragmentShader, 1, fs, NULL);
	glCompileShader(vertexShader);
	checkShader(vertexShader);

	glCompileShader(fragmentShader);
	checkShader(fragmentShader);

	unsigned int textureProgram = glCreateProgram();
	glAttachShader(textureProgram, vertexShader);
	glAttachShader(textureProgram, fragmentShader);
	glLinkProgram(textureProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return textureProgram;
}

bool checkShader(unsigned int shader) {
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        std::cout<<log;
        return false;
    }
    return true;
}