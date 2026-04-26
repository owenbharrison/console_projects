#pragma once
#ifndef PARTICLE_STRUCT_H
#define PARTICLE_STRUCT_H

#include "cmn/float2.h"

struct Particle {
	cmn::float2 pos, vel, acc;
	float age=0, lifespan=0;
	bool seed=false;
	float r=0, g=0, b=0;

	Particle(cmn::float2 p, cmn::float2 v, float l, float _r, float _g, float _b) {
		pos=p;
		vel=v;
		lifespan=l;
		r=_r;
		g=_g;
		b=_b;
	}

	void applyForce(const cmn::float2& f) {
		acc+=f;
	}

	void update(float dt) {
		vel+=dt*acc;
		pos+=dt*vel;
		acc*=0;

		age+=dt;
	}

	bool isDead() const {
		return age>lifespan;
	}
};
#endif