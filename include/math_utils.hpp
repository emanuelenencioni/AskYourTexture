#include <math.h>

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


#endif