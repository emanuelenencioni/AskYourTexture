#include <math.h>
#include <float.h>
#include <cmath>

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

void inline normalize(float v[3]){
	float len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	if (len == 0.0f) return;   // zero-vector decision (M3.0): preserve input, avoid NaN poisoning
	v[0] /= len; v[1]/=len; v[2]/=len;
}

// a × b — promoted from Camera::cross (M3.0) so math is testable outside the class
void inline cross(float out[3], const float a[3], const float b[3]){
	out[0] = a[1]*b[2]-a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}


void inline computeTangent(const float p0[3], const float p1[3], const float p2[3],
                           const float uv0[2], const float uv1[2], const float uv2[2],
                           float outT[3], float outB[3]) {
	float E1[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
	float E2[3] = {p2[0]-p0[0], p2[1]-p0[1], p2[2]-p0[2]};
	float dU1[2] = {uv1[0]-uv0[0], uv1[1]-uv0[1]};
	float dU2[2] = {uv2[0]-uv0[0], uv2[1]-uv0[1]};
	
	float det = dU1[0]*dU2[1] - dU1[1]*dU2[0];
	if(std::fabs(det) < FLT_EPSILON) {
		// degenerate UVs → fallback frame: E1/E2 lie in the triangle plane,
		// so they are valid in-plane tangents for ANY surface orientation
		for(int i=0; i<3; i++) { outT[i] = E1[i]; outB[i] = E2[i]; }
		normalize(outT);
		normalize(outB);
		return;
	}

	float r = 1/det;
	for(int i=0; i<3; i++) outT[i] = r*(E1[i]*dU2[1]-E2[i]*dU1[1]);
	for(int i=0; i<3; i++) outB[i] = r*(E2[i]*dU1[0]-E1[i]*dU2[0]);
	normalize(outT);
	normalize(outB);
}


#endif