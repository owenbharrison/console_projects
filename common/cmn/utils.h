#pragma once
#ifndef CMN_UTILS_H
#define CMN_UTILS_H

//for rand
#include <cstdlib>

namespace cmn {
	static constexpr float Pi=3.1415927f;
	
	//clever default param placement:
	//random()=0-1
	//random(a)=0-a
	//random(a, b)=a-b
	float randFloat(float b=1, float a=0) {
		static const float rand_max=RAND_MAX;
		float t=std::rand()/rand_max;
		return a+t*(b-a);
	}
}
#endif