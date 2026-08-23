#include "shadow.hpp"

Shadow::Shadow(const std::vector<float>& points, const float lightPos[3], float floorY): floorY(floorY+0.001){
    
    for(int i=0; i<3; i++){
        this->lightPos[i] = lightPos[i];
    }
    
    for(float p : points) modelPoints.push_back(p);
    
    hullVertices.clear(); 
    projectToFloor();
    grahamScan();
    
}
void Shadow::setupGL() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, hullVertices.size() * sizeof(float),
                 hullVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9*sizeof(float), (void*)24);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);   
}                
void Shadow::update(const float  lightPos[3]) {
    for(int i=0; i<3; i++) this->lightPos[i] = lightPos[i];

    hullVertices.clear(); 
    projectToFloor();
    grahamScan();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, hullVertices.size()*sizeof(float), hullVertices.data(), GL_DYNAMIC_DRAW); 
}                
void Shadow::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, hullVertices.size()/9);
}       

/**
 * @brief Projecting the modelPoints to floor respectto light position
 * 
 */
void Shadow::projectToFloor() {
    float t,x,z;
    for(int i=0; i<modelPoints.size(); i+=3){
        t = (floorY - lightPos[1]) / (modelPoints[i+1] - lightPos[1]);
        x = lightPos[0] + t*(modelPoints[i]-lightPos[0]);
        z = lightPos[2] + t*(modelPoints[i+2]-lightPos[2]);

        hullVertices.insert(hullVertices.end(), {x,floorY, z, 0.2, 0.2, 0.2, 0, 1, 0}); // numbers: RGB, normal
    }
}


void Shadow::grahamScan() {
    int n = hullVertices.size() / 9;
    if (n < 3) return;                      // degenerate

    int anchor = 0;
    for (int i = 1; i < n; ++i) {
        float zi = hullVertices[i*9+2],  z0 = hullVertices[anchor*9+2];
        float xi = hullVertices[i*9],    x0 = hullVertices[anchor*9];
        if (zi < z0 || (zi == z0 && xi < x0)) anchor = i;
    }

    // 2. collect the OTHER indices and sort by angle around the anchor
    std::vector<int> idx;
    for (int i = 0; i < n; ++i) if (i != anchor) idx.push_back(i);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        float az = atan2(hullVertices[a*9+2] - hullVertices[anchor*9+2],
                         hullVertices[a*9]   - hullVertices[anchor*9]);
        float bz = atan2(hullVertices[b*9+2] - hullVertices[anchor*9+2],
                         hullVertices[b*9]   - hullVertices[anchor*9]);
        return az < bz;
    });

    // 3. walk with a left-turn stack (vector<int>, not stack)
    std::vector<int> hull;
    hull.push_back(anchor);
    for (int p : idx) {
        while (hull.size() >= 2 && cross(hull[hull.size()-2], hull[hull.size()-1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    // 4. rewrite hullVertices to keep ONLY hull points, CCW, 9 floats each
    std::vector<float> kept;
    for(int h : hull){
        for(int k=0; k<9; k++){
            kept.push_back(hullVertices[h*9+k]);
        }
    }
    hullVertices = std::move(kept);
}

/**
 * @brief 3 indices as input for cross-product
 * 
 * @param a idx
 * @param b idx
 * @param c idx
 * @return float 
 */
float Shadow::cross(int a, int b, int c) {
    //(bx−ax)·(cz−az) − (bz−az)·(cx−ax)
    return (hullVertices[b*9]-hullVertices[a*9])*(hullVertices[c*9+2]-hullVertices[a*9+2]) - 
    (hullVertices[b*9+2]-hullVertices[a*9+2])*(hullVertices[c*9]-hullVertices[a*9]);
    
}