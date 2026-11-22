#include "showcase.hpp"
#include <glm/gtc/matrix_transform.hpp>

ShowcaseController::ShowcaseController()
    : currentMode(ShowcaseMode::Sphere)
    , targetMode(ShowcaseMode::Sphere)
    , transitionProgress(1.0f)
    , transitionDuration(0.35f)
    , currentScale(1.0f)
    , objRotAngle(0.0f)
    , turntableActive(false)
{
}

ShowcaseController::~ShowcaseController() {
}

void ShowcaseController::requestMode(ShowcaseMode mode) {
    if (mode != targetMode) {
        transitionProgress = 0.0f;
        targetMode = mode;
    }
}

void ShowcaseController::update(float dt) {
    if(turntableActive){ 
    objRotAngle += 0.5f * dt;
    }

    if (transitionProgress < 1.0) {
        transitionProgress += dt / transitionDuration;

        if (transitionProgress >= 0.5f && currentMode != targetMode) {
            currentMode = targetMode;
        }
        
        if (transitionProgress < 0.5f) {
            currentScale = 1.0f - (2.0f * transitionProgress);
        }
        else if (transitionProgress < 1.0f)
            currentScale = 2.0f * (transitionProgress - 0.5f);
        else {
            transitionProgress = 1.0f;
            currentScale = 1.0f;
        }
    }
}

void ShowcaseController::toggleTurntable() {
    turntableActive = !turntableActive;
}

ShowcaseMode ShowcaseController::getActiveMode() const {
    return currentMode;
}

glm::mat4 ShowcaseController::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, objRotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(currentScale));
    return model;
}

float ShowcaseController::getCurrentScale() const {
    return currentScale;
}

bool ShowcaseController::isTurntableEnabled() const {
    return turntableActive;
}

