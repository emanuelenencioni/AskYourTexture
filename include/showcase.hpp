

#ifndef SHOWCASE_HPP
#define SHOWCASE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum class ShowcaseMode { Sphere = 0, Cube = 1, FloorOnly = 2 };

class ShowcaseController {
public:
    ShowcaseController();
    ~ShowcaseController();

    void requestMode(ShowcaseMode mode);
    void update(float dt);
    void toggleTurntable();

    ShowcaseMode getActiveMode() const;
    glm::mat4 getModelMatrix() const;
    float getCurrentScale() const;
    bool isTurntableEnabled() const;

private:
    ShowcaseMode currentMode;
    ShowcaseMode targetMode;

    float transitionProgress; // 0.0 -> 1.0 (1.0 = settled/idle)
    const float transitionDuration; // animation duration in seconds
    float currentScale;
    float objRotAngle; // turntable spin angle in radians
    bool turntableActive;
};

#endif
