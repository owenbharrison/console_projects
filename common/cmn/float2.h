#pragma once
#ifndef CMN_FLOAT2_STRUCT_H
#define CMN_FLOAT2_STRUCT_H

//for sqrt
#include <cmath>

namespace cmn {
	struct float2 {
		float x=0, y=0;

		float2() {}
		float2(float x_, float y_) { x=x_, y=y_; }
		float2(const float2& v) { x=v.x, y=v.y; }

		float2& operator=(const float2& v)=default;

		float2 operator-() const { return {-x, -y}; }
		float2 operator+(const float2& v) const { return {x+v.x, y+v.y}; }
		float2 operator+(float s) const { return operator+({s, s}); }
		float2 operator-(const float2& v) const { return {x-v.x, y-v.y}; }
		float2 operator-(float s) const { return operator-({s, s}); }
		float2 operator*(const float2& v) const { return {x*v.x, y*v.y}; }
		float2 operator*(float s) const { return operator*({s, s}); }
		float2 operator/(const float2& v) const { return {x/v.x, y/v.y}; }
		float2 operator/(float s) const { return operator/({s, s}); }

		//i never really use these, but whatever
		float2& operator+=(const float2& v) { *this=*this+v; return*this; }
		float2& operator+=(float v) { *this=*this+v; return*this; }
		float2& operator-=(const float2& v) { *this=*this-v; return*this; }
		float2& operator-=(float v) { *this=*this-v; return*this; }
		float2& operator*=(const float2& v) { *this=*this*v; return*this; }
		float2& operator*=(float v) { *this=*this*v; return*this; }
		float2& operator/=(const float2& v) { *this=*this/v; return*this; }
		float2& operator/=(float v) { *this=*this/v; return*this; }
	};
}

cmn::float2 operator+(float s, const cmn::float2& v) { return v+s; }
cmn::float2 operator-(float s, const cmn::float2& v) { return -v+s; }
cmn::float2 operator*(float s, const cmn::float2& v) { return v*s; }
cmn::float2 operator/(float s, const cmn::float2& v) { return {s/v.x, s/v.y}; }

float dot(const cmn::float2& a, const cmn::float2& b) {
	return a.x*b.x+a.y*b.y;
}

float length(const cmn::float2& a) {
	return std::sqrt(dot(a, a));
}

cmn::float2 normalize(const cmn::float2& a) {
	return a/length(a);
}
#endif