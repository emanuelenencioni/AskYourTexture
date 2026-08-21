#include <glad/glad.h> //glad has some deps on GLFW so ALWAYS BEFORE
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <vector>

#include "cube.hpp"

void perspective(float* m, float fovRadians, float aspect, float near, float far);
//just an helper
void mult4x4(float* out, const float* a, const float* b);
void normalize(float v[3]);

int main(){
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800,600, "hello world", NULL, NULL);
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

	const char* vertexShaderSource = R"(
	#version 330 core
	
	uniform mat4 transform; // for the camera rotation

	layout (location = 0) in vec3 aPos; // posizione in ingresso letteralmente "in"
	layout (location = 1) in vec3 aColor;
	layout (location = 2) in vec3 aNormal;
	uniform vec3 lightPos;
	out float ourBrightness; 
	out vec3 ourColor;

	void main()
	{
		gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0f);   // coord. omogenee.
		ourBrightness =0.3 + 0.7*max(dot(aNormal, normalize(lightPos-aPos)), 0.0);
		ourColor = aColor;
	}
	)";

	const char* fragmentShaderSource = R"(
	
	#version 330 core
	in vec3 ourColor;
	in float ourBrightness;
	
	out vec4 FragColor; // vettore di output = colore
	void main() {
		FragColor = vec4(ourColor * ourBrightness, 1.0);  
	})";

	// creazione shaders
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	// passo il prog.
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	
	// compilazione shaders
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//a quanto pare, una volta messi nel programma, gli shader non servono più (singoli)
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	
	

// ----- STUFF IN THE SCENE -----

// CUBE now has a class
Cube cube;
cube.setupGL();


float floorVertices[] = { // FLOOR
    // normal (0,1,0) — gray
    -4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
     4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
     4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
    -4.0f, -1.2f, -4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
    -4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
     4.0f, -1.2f,  4.0f,  0.5f, 0.5f, 0.5f,  0,1,0,
};
	unsigned int VBOFloor;
	glGenBuffers(1, &VBOFloor);
	glBindBuffer(GL_ARRAY_BUFFER, VBOFloor);

	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	
	unsigned int VAOFloor;
	glGenVertexArrays(1, &VAOFloor);
	glBindVertexArray(VAOFloor); 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)12);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)24);
	glEnableVertexAttribArray(2);


	float window_x_size = 1280;
	float window_y_size = 720;

	glViewport(0,0,window_x_size,window_y_size);
	glEnable(GL_DEPTH_TEST);

	float aspect_ratio = window_x_size / window_y_size;
	//rotation data
	float m[16];
	float fov = M_PI/3;
	float elev = -10*M_PI/180;//M_PI/6;
	float transform[16];
	float view[16] = {0}; view[0]=1; view[5]=1; view[10]=1; view[15]=1;
	float projection[16];
	perspective(projection,fov,aspect_ratio,0.1, 100);
	float R = 5.0f;
	float target[3] = {0};
	float f[3];
	float r[3];
	float up[3] = {0,1,0};
	float s[3];
	GLint loc = glGetUniformLocation(shaderProgram, "transform");

	// light source!
	float lightPos[3] = {5.0f, 5.0f, 5.0f};
	GLint lightLoc = glGetUniformLocation(shaderProgram, "lightPos");
	
	// loop
	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);
	//  ----- ORBITING VIEW CALCULATION -----
		float t = glfwGetTime();
		//camera orbiting
		float eye[3] = { 
			R*cos(t)*cos(elev), 
			R*sin(elev), 
			R*sin(t)*cos(elev)
		};

		f[0] = -eye[0]; f[1] =  -eye[1]; f[2] = -eye[2]; 
		normalize(f);
		// up x f
		r[0] = up[1]*f[2] -up[2]*f[1];
		r[1] = up[2]*f[0] - up[0]*f[2];
		r[2] = up[0]*f[1] - up[1]*f[0];
		// f x r
		s[0] = f[1]*r[2] - f[2]*r[1];

		s[1] = f[2]*r[0] - f[0]*r[2];
		s[2] = f[0]*r[1] - f[1]*r[0];

		view[0]=r[0]; view[4]=r[1]; view[8]=r[2];
		view[1]=s[0]; view[5]=s[1]; view[9]=s[2];
		view[2]=-f[0]; view[6]=-f[1]; view[10]=-f[2];
		view[3]=0; view[7]=0; view[11]=0;
		view[12] = -( r[0]*eye[0] + r[1]*eye[1] + r[2]*eye[2] );   // -dot(r, eye)
		view[13] = -( s[0]*eye[0] + s[1]*eye[1] + s[2]*eye[2] );   // -dot(s, eye)
		view[14] =  ( f[0]*eye[0] + f[1]*eye[1] + f[2]*eye[2] );   // +dot(f, eye)
		
		mult4x4(transform, projection, view);
	
		glUniformMatrix4fv(loc, 1, GL_FALSE, transform);

		cube.draw();
	// ----- LIGHT CALCULATION -----
		glUniform3fv(lightLoc, 1, lightPos); 		
		
		glBindVertexArray(VAOFloor);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}



void perspective(float* m, float fovRadians, float aspect, float near, float far) {
	
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

void normalize(float v[3]){
	float len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	v[0] /= len; v[1]/=len; v[2]/=len;
}
