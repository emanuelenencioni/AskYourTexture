/**
 * @file pbr.hpp
 * @author your name (you@domain.com)
 * @brief function available as a c++ ref for the GLSL functions
 * @version 0.1
 * @date 2026-10-06
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef PBR_H
#define PBR_H

#include <cmath>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Standard dielectric base reflectivity (LearnOpenGL PBR Theory)
const float DIELECTRIC_F0 = 0.04f;

// k remap for DIRECT lighting only: k = (r + 1)^2 / 8
float inline k_direct(float roughness) {
	return std::pow(roughness+1, 2)/8; 
}

void inline f0_metallicMix(const float albedo[3], float metallic, float f0[3]) {
	for (int i = 0; i < 3; ++i) 
		f0[i] = DIELECTRIC_F0 + metallic*(albedo[i] - DIELECTRIC_F0); 
}

// Fresnel–Schlick approximation: F = F0 + (1 − F0) * (1 − VdotH)^5
void inline f_schlick(float VdotH, const float f0[3], float fresnel[3]) {
	for (int i = 0; i < 3; ++i) fresnel[i] = f0[i]+(1-f0[i])*std::pow((1-VdotH),5);
}

// GGX / Trowbridge-Reitz normal distribution (alpha = roughness^2)
float inline d_ggx(float NdotH, float roughness) {
	float alpha = std::pow(roughness,2);
	return std::pow(alpha,2) / (M_PI*(std::pow((std::pow(NdotH,2) * (std::pow(alpha,2)-1)+1), 2))); 
}

// G1(X) = X / (X * (1 − k) + k)
float inline g_smith(float NdotV, float NdotL, float roughness) {
	float k = k_direct(roughness);
	float G1_v = NdotV / (NdotV*(1-k) + k);
	float G1_l = NdotL / (NdotL*(1-k) + k);

	return G1_v * G1_l; 
}
#endif
