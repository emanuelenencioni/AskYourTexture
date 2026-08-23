#ifndef SHADOW_H
#define SHADOW_H

#include <math.h>
#include <glad/glad.h>
#include <utility>
#include <vector>
#include <algorithm>

class Shadow {
    public:
        Shadow(const std::vector<float>& points, const float lightPos[3], float floorY);
        void setupGL();                
        void update(const float lightPos[3]);                
        void draw();              
    private:
        void projectToFloor();          
        void grahamScan();   
        float cross(int a, int b, int c);
        
        std::vector<float> modelPoints;
        std::vector<float> hullVertices;

        float lightPos[3];
        float floorY;

        unsigned int VBO, VAO;
};

#endif