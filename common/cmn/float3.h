#pragma once
#ifndef CMN_FLOAT3_STRUCT_H
#define CMN_FLOAT3_STRUCT_H

//for sqrt & abs
#include <cmath>

namespace cmn {
	struct float3 {
		float x=0, y=0, z=0;

		float3() {}
		float3(float x_, float y_, float z_) { x=x_, y=y_, z=z_; }
		float3(const float3& v) { x=v.x, y=v.y, z=v.z; }

		float3& operator=(const float3& v)=default;

		float3 operator-() const { return {-x, -y, -z}; }
		float3 operator+(const float3& v) const { return {x+v.x, y+v.y, z+v.z}; }
		float3 operator+(float s) const { return operator+({s, s, s}); }
		float3 operator-(const float3& v) const { return {x-v.x, y-v.y, z-v.z}; }
		float3 operator-(float s) const { return operator-({s, s, s}); }
		float3 operator*(const float3& v) const { return {x*v.x, y*v.y, z*v.z}; }
		float3 operator*(float s) const { return operator*({s, s, s}); }
		float3 operator/(const float3& v) const { return {x/v.x, y/v.y, z/v.z}; }
		float3 operator/(float s) const { return operator/({s, s, s}); }

		//i never really use these, but whatever
		float3& operator+=(const float3& v) { *this=*this+v; return*this; }
		float3& operator+=(float v) { *this=*this+v; return*this; }
		float3& operator-=(const float3& v) { *this=*this-v; return*this; }
		float3& operator-=(float v) { *this=*this-v; return*this; }
		float3& operator*=(const float3& v) { *this=*this*v; return*this; }
		float3& operator*=(float v) { *this=*this*v; return*this; }
		float3& operator/=(const float3& v) { *this=*this/v; return*this; }
		float3& operator/=(float v) { *this=*this/v; return*this; }
	};
}

cmn::float3 operator+(float s, const cmn::float3& v) { return v+s; }
cmn::float3 operator-(float s, const cmn::float3& v) { return -v+s; }
cmn::float3 operator*(float s, const cmn::float3& v) { return v*s; }
cmn::float3 operator/(float s, const cmn::float3& v) { return {s/v.x, s/v.y, s/v.z}; }

float dot(const cmn::float3& a, const cmn::float3& b) {
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

float length(const cmn::float3& a) {
	return std::sqrt(dot(a, a));
}

cmn::float3 normalize(const cmn::float3& a) {
	return a/length(a);
}

cmn::float3 cross(const cmn::float3& a, const cmn::float3& b) {
	return {
		a.y*b.z-a.z*b.y,
		a.z*b.x-a.x*b.z,
		a.x*b.y-a.y*b.x
	};
}
#endif