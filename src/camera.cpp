#include "camera.hpp"

Camera::Camera(float pos[3], float yawDeg, float pitchDeg)
: yaw(yawDeg*M_PI/180), pitch(pitchDeg*M_PI/180), firstMouse(1) {
    for(int i=0; i<3; i++) position[i] = pos[i];

    sensitivity = 0.003;
    speed = 5;
}

void Camera::processMouse(double xpos, double ypos) {
    if(firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    double dx = xpos - lastX;
    double dy = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    yaw -= dx * sensitivity;
    pitch += - dy * sensitivity; // here for the Y sign
    
    // clambing, avoiding problems with PI/2.
    
    pitch = pitch <= eig_nine ? pitch : eig_nine;
    pitch = pitch >= -eig_nine ? pitch : -eig_nine;
}
void Camera::processKeyboard(int key, float dt) {
    float front[3];
    float right[3];
    getFront(front);
    getRight(right);
    float step = speed * dt;

    switch(key){
        case GLFW_KEY_W:
            for(int i=0; i<3; i++){
            position[i] += front[i]*step;
            }
            break;

        case GLFW_KEY_S:
            for(int i=0; i<3; i++){
            position[i] -= front[i]*step;
            }
            break;
        case GLFW_KEY_D:
            for(int i=0; i<3; i++){
            position[i] += right[i]*step;
            }
            break;
        case GLFW_KEY_A:
            for(int i=0; i<3; i++){
                position[i] -= right[i]*step;
            }
            break;
        default: break;
    }

}
void Camera::getViewMatrix(float out[16]) {
    float f[3]; //front
    float s[3], r[3];
    getFront(f);
    normalize(f);
    // up x f
    r[0] = worldUp[1]*f[2] -worldUp[2]*f[1];
    r[1] = worldUp[2]*f[0] - worldUp[0]*f[2];
    r[2] = worldUp[0]*f[1] - worldUp[1]*f[0];
    // f x r
    s[0] = f[1]*r[2] - f[2]*r[1];

    s[1] = f[2]*r[0] - f[0]*r[2];
    s[2] = f[0]*r[1] - f[1]*r[0];

    out[0]=r[0]; out[4]=r[1]; out[8]=r[2];
    out[1]=s[0]; out[5]=s[1]; out[9]=s[2];
    out[2]=-f[0]; out[6]=-f[1]; out[10]=-f[2];
    out[3]=0; out[7]=0; out[11]=0;
    out[12] = -( r[0]*position[0] + r[1]*position[1] + r[2]*position[2] );   // -dot(r, position)
    out[13] = -( s[0]*position[0] + s[1]*position[1] + s[2]*position[2] );   // -dot(s, position)
    out[14] =  ( f[0]*position[0] + f[1]*position[1] + f[2]*position[2] );   // +dot(f, eye)
    
}
// TODO: transplant your f/r/s code from main.cpp:182-200



void Camera::getFront(float out[3]){ 
    // trig: (cosY*cosP, sinP, sinY*cosP)


    out[0] = cos(yaw)*cos(pitch);
    out[1] =  sin(pitch); 
    out[2] = sin(yaw)*cos(pitch);
    normalize(out);
}
void Camera::getRight(float out[3]) {
    float front[3];
    getFront(front);
    cross(out, worldUp, front);

}  // front × worldUp — you already wrote cross products
void Camera::getUp(float out[3]) {
    float front[3], right[3];
    getFront(front);
    getRight(right);
    cross(out,front,right);
    normalize(out);
}     // right × front