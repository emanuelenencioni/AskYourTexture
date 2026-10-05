#include "doctest.h"
// NOTE: no DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN here — test_math.cpp is the
// single main TU of the `tests` target; this file only adds TEST_CASEs.
#include "pbr.hpp"
#include <cmath>

// M1 Task 2 — property contract for the Cook-Torrance BRDF building blocks.
// These tests are the SPEC; pbr.hpp must satisfy all of them before any
// GLSL port happens (testability was the whole point of the C++ mirror).

static const float W = 0.7f, G = 0.55f, B = 0.4f;   // an arbitrary albedo
static const float EPS = 1e-4f;

TEST_CASE("f0_metallicMix") {
    SUBCASE("dielectric: metallic=0 → standard 0.04 for all channels") {
        float f0[3];
        f0_metallicMix((float[3]){W, G, B}, 0.0f, f0);
        CHECK(f0[0] == doctest::Approx(0.04f));
        CHECK(f0[1] == doctest::Approx(0.04f));
        CHECK(f0[2] == doctest::Approx(0.04f));
    }
    SUBCASE("metal: metallic=1 → F0 IS the albedo") {
        float f0[3];
        f0_metallicMix((float[3]){W, G, B}, 1.0f, f0);
        CHECK(f0[0] == doctest::Approx(W));
        CHECK(f0[1] == doctest::Approx(G));
        CHECK(f0[2] == doctest::Approx(B));
    }
    SUBCASE("half metal → linear blend") {
        float f0[3];
        f0_metallicMix((float[3]){1, 0, 1}, 0.5f, f0);
        CHECK(f0[0] == doctest::Approx((0.04f + 1.0f) * 0.5f));
        CHECK(f0[1] == doctest::Approx(0.04f * 0.5f));
    }
}

TEST_CASE("f_schlick — Fresnel endpoints") {
    const float f0[3] = {0.04f, 0.04f, 0.04f};
    SUBCASE("normal incidence: VdotH=1 → F == F0") {
        float f[3];
        f_schlick(1.0f, f0, f);
        CHECK(f[0] == doctest::Approx(0.04f).epsilon(EPS));
    }
    SUBCASE("grazing incidence: VdotH=0 → F → 1 (the wet-road effect)") {
        float f[3];
        f_schlick(0.0f, f0, f);
        CHECK(f[0] == doctest::Approx(1.0f).epsilon(EPS));
    }
    SUBCASE("monotonic in VdotH") {
        float fa[3], fb[3];
        f_schlick(0.3f, f0, fa);
        f_schlick(0.7f, f0, fb);
        CHECK(fb[0] < fa[0]);
    }
}

TEST_CASE("d_ggx — GGX distribution") {
    SUBCASE("analytic value at peak: NdotH=1, roughness=0.25 → 1/(pi*alpha^2)") {
        // alpha = 0.0625, (NdotH^2*(a^2-1)+1)^2 = a^4 → D = 1/(pi*a^2) ≈ 81.49
        CHECK(d_ggx(1.0f, 0.25f) == doctest::Approx(81.49f).epsilon(1e-3f));
    }
    SUBCASE("rougher surface → lower, flatter peak") {
        CHECK(d_ggx(1.0f, 0.25f) > d_ggx(1.0f, 0.5f));
        CHECK(d_ggx(1.0f, 0.5f) > d_ggx(1.0f, 1.0f));
    }
    SUBCASE("non-negative for grazing alignment") {
        CHECK(d_ggx(0.0f, 0.5f) >= 0.0f);
    }
    SUBCASE("energy: integrates to ≈ 1 over the hemisphere (numerical, coarse grid)") {
        // ∫Ω D(NdotH) dω over the upper hemisphere with N = +Z, uniform in (theta, phi)
        const int N = 400;   // theta steps; phi steps proportional (sin-weighted)
        float sum = 0.0f;
        for (int i = 0; i < N; ++i) {
            float theta = (M_PI / 2.0f) * (i + 0.5f) / N;              // 0..pi/2
            float cosT = std::cos(theta), sinT = std::sin(theta);
            for (int j = 0; j < N; ++j) {
                float phi = (2.0f * M_PI) * (j + 0.5f) / N;
                float h[3] = {sinT * std::cos(phi), sinT * std::sin(phi), cosT};
                float NdotH = h[2];
                // solid angle cell = sinT * dTheta * dPhi
                sum += d_ggx(NdotH, 0.35f) * cosT * sinT * (M_PI / 2.0f / N) * (2.0f * M_PI / N);
            }
        }
        CHECK(sum == doctest::Approx(1.0f).epsilon(0.05f));
    }
}

TEST_CASE("g_smith — geometry/masking") {
    SUBCASE("bounded: G <= 1 across a sweep of inputs") {
        for (float r = 0.1f; r <= 1.0f; r += 0.3f) {
            for (float nv = 0.05f; nv <= 1.0f; nv += 0.19f) {
                for (float nl = 0.05f; nl <= 1.0f; nl += 0.23f) {
                    CHECK(g_smith(nv, nl, r) <= 1.0f);
                    CHECK(g_smith(nv, nl, r) >= 0.0f);
                }
            }
        }
    }
    SUBCASE("head-on light+view, low roughness → nearly no occlusion") {
        CHECK(g_smith(1.0f, 1.0f, 0.1f) > 0.95f);
    }
    SUBCASE("grazing view → strong masking (the scratched-surface effect)") {
        CHECK(g_smith(0.01f, 1.0f, 0.5f) < g_smith(0.9f, 1.0f, 0.5f));
    }
    SUBCASE("light grazing the surface → shadowing") {
        CHECK(g_smith(1.0f, 0.01f, 0.5f) < g_smith(1.0f, 0.9f, 0.5f));
    }
}

TEST_CASE("k_direct — the energy split remap") {
    SUBCASE("analytic: (r+1)^2/8 at r=0.5 → 0.28125") {
        CHECK(k_direct(0.5f) == doctest::Approx(0.28125f).epsilon(EPS));
    }
    SUBCASE("grows with roughness, stays in [1/8, 1/4]") {
        CHECK(k_direct(0.0f) == doctest::Approx(0.125f).epsilon(EPS));
        CHECK(k_direct(1.0f) == doctest::Approx(0.5f).epsilon(EPS));
        CHECK(k_direct(0.8f) > k_direct(0.2f));
    }
}

TEST_CASE("energy conservation — the specular/diffuse split") {
    // Specular (D*G*F / denominators) may never receive more energy than
    // (1 − F)(1 − metallic) leaves for it. Indirect contract check: F(0)=F0
    // and kD = (1−F0)(1−metallic) means kD + kS ≤ 1 at normal incidence.
    SUBCASE("dielectric at normal incidence: kD = 1 − F0") {
        float f0[3];
        f0_metallicMix((float[3]){W, G, B}, 0.0f, f0);
        float f[3];
        f_schlick(1.0f, f0, f);
        float kD = (1.0f - f[0]) * (1.0f - 0.0f);
        CHECK(kD == doctest::Approx(0.96f).epsilon(EPS));
    }
    SUBCASE("full metal: kD = 0 — no diffuse, all specular") {
        float f0[3];
        f0_metallicMix((float[3]){W, G, B}, 1.0f, f0);
        float f[3];
        f_schlick(1.0f, f0, f);
        float kD = (1.0f - f[0]) * (1.0f - 1.0f);
        CHECK(kD == doctest::Approx(0.0f).epsilon(EPS));
    }
}
