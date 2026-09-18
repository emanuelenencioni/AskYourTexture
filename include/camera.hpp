#include <math.h>
#include <GLFW/glfw3.h>

#include "math_utils.hpp"

#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public:
    Camera(float pos[3], float yawDeg, float pitchDeg); // TODO: what yaw/pitch reproduce your old orbit start?

    void processMouse(double xpos, double ypos);
    void processKeyboard(int key, float dt);

    void getViewMatrix(float out[16]); // TODO: transplant your f/r/s code from main.cpp:182-200

private:
    // TODO — state you OWN (can't be derived);
    float   position[3]; 
    float yaw, pitch;

    const float worldUp[3] = {0,1,0};
    const double eig_nine = 89*M_PI/180; // 89° in rad
    float speed, sensitivity;

    float lastX, lastY;
    bool firstMouse;
    // TODO — derived helpers (recompute, never store):
    void getFront(float out[3]);  // trig: (cosY*cosP, sinP, sinY*cosP)
    void getRight(float out[3]);  // front × worldUp — you already wrote cross products
    void getUp(float out[3]);     // right × front
};


#endif