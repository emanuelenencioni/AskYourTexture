#include <math.h>

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

void inline normalize(float v[3]){
	float len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	v[0] /= len; v[1]/=len; v[2]/=len;
}


#endif