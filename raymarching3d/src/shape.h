#pragma once
#ifndef SHAPE_STRUCT_H
#define SHAPE_STRUCT_H

#include "cmn/float3.h"
#include "cmn/float2.h"

//for min, max, & clamp
#include <algorithm>

cmn::float3 abs(const cmn::float3& a) {
	return {
		std::abs(a.x),
		std::abs(a.y),
		std::abs(a.z)
	};
}

cmn::float3 min(const cmn::float3& a, const cmn::float3& b) {
	return {
		std::min(a.x, b.x),
		std::min(a.y, b.y),
		std::min(a.z, b.z)
	};
}

cmn::float3 max(const cmn::float3& a, const cmn::float3& b) {
	return {
		std::max(a.x, b.x),
		std::max(a.y, b.y),
		std::max(a.z, b.z)
	};
}

float sign(float v) {
	return v==0?0:v>0?1:-1;
}

float dot2(const cmn::float3& a) {
	return dot(a, a);
}

//https://iquilezles.org/articles/distfunctions/
float sdSphere(cmn::float3 p, float r) {
	return length(p)-r;
}

//https://www.shadertoy.com/view/4sXXRN
float sdTriangle(cmn::float3 p, cmn::float3 a, cmn::float3 b, cmn::float3 c) {
	cmn::float3 ba=b-a, pa=p-a;
	cmn::float3 cb=c-b, pb=p-b;
	cmn::float3 ac=a-c, pc=p-c;
	cmn::float3 nor=cross(ba, ac);

	cmn::float3 zero, one(1, 1, 1);
	return std::sqrt(
		(sign(dot(cross(ba, nor), pa))+
			sign(dot(cross(cb, nor), pb))+
			sign(dot(cross(ac, nor), pc))<2)
		?
		std::min(std::min(
			dot2(ba*std::clamp(dot(ba, pa)/dot2(ba), 0.f, 1.f)-pa),
			dot2(cb*std::clamp(dot(cb, pb)/dot2(cb), 0.f, 1.f)-pb)),
			dot2(ac*std::clamp(dot(ac, pc)/dot2(ac), 0.f, 1.f)-pc))
		:
		dot(nor, pa)*dot(nor, pa)/dot2(nor));
}

//https://www.shadertoy.com/view/3ljcRh
float sdBoxFrame(cmn::float3 p, cmn::float3 b, float e) {
	p=abs(p)-b;
	cmn::float3 q=abs(p+e)-e;
	cmn::float3 zero;
	float d0=length(max(zero, cmn::float3(p.x, q.y, q.z)))+std::min(0.f, std::max(p.x, std::max(q.y, q.z)));
	float d1=length(max(zero, cmn::float3(q.x, p.y, q.z)))+std::min(0.f, std::max(q.x, std::max(p.y, q.z)));
	float d2=length(max(zero, cmn::float3(q.x, q.y, p.z)))+std::min(0.f, std::max(q.x, std::max(q.y, p.z)));
	return std::min(std::min(d0, d1), d2);
}

//https://iquilezles.org/articles/distfunctions/
float sdTorus(cmn::float3 p, float R, float r) {
	cmn::float2 p_xy(p.x, p.y);
	cmn::float2 q(length(p_xy)-R, p.z);
	return length(q)-r;
}

//https://www.shadertoy.com/view/wsSGDG
float sdOctahedron(cmn::float3 p, float s) {
	p=abs(p);
	float m=p.x+p.y+p.z-s;
	cmn::float3 q;
	if(3*p.x<m) q=cmn::float3(p.x, p.y, p.z);
	else if(3*p.y<m) q=cmn::float3(p.y, p.z, p.x);
	else if(3*p.z<m) q=cmn::float3(p.z, p.x, p.y);
	else return m*.5773503f;

	float k=std::clamp(.5f*(q.z-q.y+s), 0.f, s);
	return length(cmn::float3(q.x, q.y-s+k, q.z-k));
}

struct Shape {
	cmn::float3 col;

	virtual float signedDist(cmn::float3 p)const=0;
};

struct Sphere : Shape {
	cmn::float3 pos;
	float rad=0;

	Sphere(cmn::float3 c, cmn::float3 p, float r) {
		col=c;
		pos=p;
		rad=r;
	}

	float signedDist(cmn::float3 p) const override {
		return sdSphere(p-pos, rad);
	}
};

struct Triangle : Shape {
	cmn::float3 p0, p1, p2;

	Triangle(cmn::float3 c, cmn::float3 _a, cmn::float3 _b, cmn::float3 _c) {
		col=c;
		p0=_a;
		p1=_b;
		p2=_c;
	}

	float signedDist(cmn::float3 p) const override {
		return sdTriangle(p, p0, p1, p2);
	}
};

struct BoxFrame : Shape {
	cmn::float3 pos, size;
	float thick=0;

	BoxFrame(cmn::float3 c, cmn::float3 p, cmn::float3 s, float t) {
		col=c;
		pos=p;
		size=s;
		thick=t;
	}

	float signedDist(cmn::float3 p) const override {
		return sdBoxFrame(p-pos, size, thick);
	}
};

struct Torus : Shape {
	cmn::float3 pos;
	float R=0, r=0;

	Torus(cmn::float3 c, cmn::float3 p, float _R, float _r) {
		col=c;
		pos=p;
		R=_R;
		r=_r;
	}

	float signedDist(cmn::float3 p) const override {
		return sdTorus(p-pos, R, r);
	}
};

struct Octahedron : Shape {
	cmn::float3 pos;
	float size=0;

	Octahedron(cmn::float3 c, cmn::float3 p, float s) {
		col=c;
		pos=p;
		size=s;
	}

	float signedDist(cmn::float3 p) const override {
		return sdOctahedron(p-pos, size);
	}
};
#endif