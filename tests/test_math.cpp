#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "math_utils.hpp"
#include <cmath>

// M3.0 — framework kata on existing math.
// NOTE: the zero-vector decision (M3.0): normalize() early-returns the input
//       (len == 0 → NaN poisoning avoided). Recorded in "normalize / zero vector" below.

TEST_CASE("normalize") {
    SUBCASE("axis vector") {
        float v[3] = {2.0f, 0.0f, 0.0f};
        normalize(v);
        CHECK(v[0] == doctest::Approx(1.0f));
        CHECK(v[1] == doctest::Approx(0.0f));
        CHECK(v[2] == doctest::Approx(0.0f));
    }
    SUBCASE("zero vector — decision: early-return preserves input") {
        float v[3] = {0.0f, 0.0f, 0.0f};
        normalize(v);   // must NOT produce NaN
        CHECK(std::isnan(v[0]) == false);
        CHECK(v[0] == doctest::Approx(0.0f));
        CHECK(v[1] == doctest::Approx(0.0f));
        CHECK(v[2] == doctest::Approx(0.0f));
    }
    SUBCASE("negative axis") {
        float v[3] = {0.0f, 0.0f, -5.0f};
        normalize(v);
        CHECK(v[0] == doctest::Approx(0.0f));
        CHECK(v[1] == doctest::Approx(0.0f));
        CHECK(v[2] == doctest::Approx(-1.0f));
    }
    SUBCASE("already unit — fixed point") {
        float v[3] = {0.0f, 1.0f, 0.0f};
        normalize(v);
        CHECK(v[0] == doctest::Approx(0.0f));
        CHECK(v[1] == doctest::Approx(1.0f));
        CHECK(v[2] == doctest::Approx(0.0f));
    }
}

TEST_CASE("cross") {
    SUBCASE("textbook identity: x̂ × ŷ = ẑ") {
        float r[3];
        cross(r, (float[3]){1,0,0}, (float[3]){0,1,0});
        CHECK(r[0] == doctest::Approx(0.0f));
        CHECK(r[1] == doctest::Approx(0.0f));
        CHECK(r[2] == doctest::Approx(1.0f));
    }
    SUBCASE("up × x (the getRight proof-of-work)") {
        float r[3];
        cross(r, (float[3]){0,1,0}, (float[3]){1,0,0});
        CHECK(r[0] == doctest::Approx(0.0f));
        CHECK(r[1] == doctest::Approx(0.0f));
        CHECK(r[2] == doctest::Approx(-1.0f));
    }
    SUBCASE("parallel → zero (degenerate frame warning)") {
        float r[3];
        cross(r, (float[3]){1,2,3}, (float[3]){2,4,6});
        CHECK(r[0] == doctest::Approx(0.0f));
        CHECK(r[1] == doctest::Approx(0.0f));
        CHECK(r[2] == doctest::Approx(0.0f));
    }
    SUBCASE("anti-parallel → zero") {
        float r[3];
        cross(r, (float[3]){1,0,0}, (float[3]){-1,0,0});
        CHECK(r[0] == doctest::Approx(0.0f));
        CHECK(r[1] == doctest::Approx(0.0f));
        CHECK(r[2] == doctest::Approx(0.0f));
    }
    SUBCASE("magnitude law |a×b| = |a||b|sin(θ), θ=90°") {
        float r[3];
        cross(r, (float[3]){1,0,0}, (float[3]){0,1,0});   // x̂ × ŷ
        float len = std::sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
        CHECK(len == doctest::Approx(1.0f));               // 1·1·sin(90°) = 1
    }
}
