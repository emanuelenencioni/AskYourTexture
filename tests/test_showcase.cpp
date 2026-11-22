#include "doctest.h"
#include "showcase.hpp"

// TDD Specification for ShowcaseController

TEST_CASE("ShowcaseController - initial state") {
    ShowcaseController controller;
    CHECK(controller.getActiveMode() == ShowcaseMode::Sphere);
    CHECK(controller.getCurrentScale() == doctest::Approx(1.0f));
    CHECK(controller.isTurntableEnabled() == false);
}

TEST_CASE("ShowcaseController - request same mode is idempotent") {
    ShowcaseController controller;
    controller.requestMode(ShowcaseMode::Sphere);
    // Advancing time should not trigger shrink/pop
    controller.update(0.1f);
    CHECK(controller.getActiveMode() == ShowcaseMode::Sphere);
    CHECK(controller.getCurrentScale() == doctest::Approx(1.0f));
}

TEST_CASE("ShowcaseController - transition lifecycle (Scale & Pop)") {
    ShowcaseController controller;
    const float duration = 0.35f;

    controller.requestMode(ShowcaseMode::Cube);

    SUBCASE("Phase 1: Shrinking old mesh") {
        // Halfway through shrinking phase (t = 0.25 * duration)
        controller.update(duration * 0.25f);
        // Still displaying Sphere while shrinking
        CHECK(controller.getActiveMode() == ShowcaseMode::Sphere);
        // Scale should be strictly decreasing between 0 and 1
        CHECK(controller.getCurrentScale() < 1.0f);
        CHECK(controller.getCurrentScale() > 0.0f);
    }

    SUBCASE("Midpoint: Mesh swap at minimum scale") {
        // Advance to the midpoint (t = 0.5 * duration)
        controller.update(duration * 0.5f);
        // Active mode must now be Cube!
        CHECK(controller.getActiveMode() == ShowcaseMode::Cube);
        // Scale should be approximately 0
        CHECK(controller.getCurrentScale() == doctest::Approx(0.0f).epsilon(0.05f));
    }

    SUBCASE("Phase 2: Popping new mesh and settling") {
        // Complete the entire transition
        controller.update(duration);
        CHECK(controller.getActiveMode() == ShowcaseMode::Cube);
        CHECK(controller.getCurrentScale() == doctest::Approx(1.0f));
    }
}

TEST_CASE("ShowcaseController - turntable toggle and rotation") {
    ShowcaseController controller;
    CHECK(controller.isTurntableEnabled() == false);

    // No rotation while disabled
    glm::mat4 m0 = controller.getModelMatrix();
    controller.update(1.0f);
    glm::mat4 m1 = controller.getModelMatrix();
    CHECK(m0 == m1);

    // Toggle on
    controller.toggleTurntable();
    CHECK(controller.isTurntableEnabled() == true);

    controller.update(0.5f);
    glm::mat4 m2 = controller.getModelMatrix();
    // Rotation must have modified the matrix
    CHECK(m1 != m2);
}
