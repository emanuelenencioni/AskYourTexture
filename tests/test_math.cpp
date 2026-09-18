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

TEST_CASE("computeTangent") {
    // Quad spec: p0=(0,0,0) p1=(1,0,0) p2=(0,0,1) — base case in all subcases below.

    SUBCASE("unit quad — UVs follow X") {
        // E1=(1,0,0) E2=(0,0,1) dU1=(1,0) dU2=(0,1) → det=1, T=E1, B=E2
        float T[3], B[3];
        float p0[3]={0,0,0}, p1[3]={1,0,0}, p2[3]={0,0,1};
        float uv0[2]={0,0}, uv1[2]={1,0}, uv2[2]={0,1};
        computeTangent(p0,p1,p2, uv0,uv1,uv2, T,B);
        CHECK(T[0] == doctest::Approx(1.0f));
        CHECK(T[1] == doctest::Approx(0.0f));
        CHECK(T[2] == doctest::Approx(0.0f));
        CHECK(B[0] == doctest::Approx(0.0f));
        CHECK(B[1] == doctest::Approx(0.0f));
        CHECK(B[2] == doctest::Approx(1.0f));
    }
    SUBCASE("UVs swapped — T follows UVs, not positions") {
        // dU1=(0,1) dU2=(1,0) → det=−1, r=−1 → T=−(−E2)=E2=(0,0,1), B=E1=(1,0,0)
        // (roles of T and B trade — the tangent frame tracks the UV layout)
        float T[3], B[3];
        float p0[3]={0,0,0}, p1[3]={1,0,0}, p2[3]={0,0,1};
        float uv0[2]={0,0}, uv1[2]={0,1}, uv2[2]={1,0};
        computeTangent(p0,p1,p2, uv0,uv1,uv2, T,B);
        CHECK(T[0] == doctest::Approx(0.0f));
        CHECK(T[1] == doctest::Approx(0.0f));
        CHECK(T[2] == doctest::Approx(1.0f));
        CHECK(B[0] == doctest::Approx(1.0f));
        CHECK(B[1] == doctest::Approx(0.0f));
        CHECK(B[2] == doctest::Approx(0.0f));
    }
    SUBCASE("degenerate UVs — fallback frame from edge directions, no NaN") {
        // Contract (revised M3.3): det==0 → T=normalize(E1), B=normalize(E2)
        // For this quad: E1=(1,0,0), E2=(0,0,1) → same numbers as the floor default,
        // but the fallback is geometric, so it stays valid on walls too.
        float T[3]={9,9,9}, B[3]={9,9,9};   // poisoned on purpose: proves the function OVERWRITES
        float p0[3]={0,0,0}, p1[3]={1,0,0}, p2[3]={0,0,1};
        float uv0[2]={0.5f,0.5f}, uv1[2]={0.5f,0.5f}, uv2[2]={0.5f,0.5f};
        computeTangent(p0,p1,p2, uv0,uv1,uv2, T,B);
        CHECK(T[0] == doctest::Approx(1.0f));
        CHECK(T[1] == doctest::Approx(0.0f));
        CHECK(T[2] == doctest::Approx(0.0f));
        CHECK(B[0] == doctest::Approx(0.0f));
        CHECK(B[1] == doctest::Approx(0.0f));
        CHECK(B[2] == doctest::Approx(1.0f));
        CHECK(std::isnan(T[0]) == false);
    }
}
