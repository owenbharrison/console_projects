#pragma once
#ifndef PARTICLE_STRUCT_H
#define PARTICLE_STRUCT_H

#include "cmn/float2.h"

//https://thecodingtrain.com/challenges/59-steering-behaviors
struct Particle {
	cmn::float2 pos, vel, acc, target;
	static const float max_speed, max_force;
	float r=0, g=0, b=0;

	Particle(cmn::float2 p, cmn::float2 t, float _r, float _g, float _b) {
		pos=p;
		target=t;
		r=_r;
		g=_g;
		b=_b;
	}

	void update(float dt) {
		//euler explicit
		vel+=dt*acc;
		pos+=dt*vel;
		acc*=0;
	}

	void applyForce(const cmn::float2& f) {
		acc+=f;
	}

	cmn::float2 getArrive(const cmn::float2& tg) const {
		cmn::float2 des=tg-pos;
		float d=length(des);
		if(d==0) return cmn::float2();

		float speed=max_speed;
		const float rad=100;
		if(d<rad) speed=d/rad*max_speed;

		//setmag to speed
		des*=speed/d;
		cmn::float2 steer=des-vel;

		//limit
		float s=length(steer);
		if(s>max_force) steer*=max_force/s;
		
		return steer;
	}

	cmn::float2 getFlee(const cmn::float2& tg) const {
		cmn::float2 des=tg-pos;
		float d=length(des);
		if(d==0) return cmn::float2();

		//opposite
		des*=-max_speed/d;
		cmn::float2 steer=des-vel;

		//limit
		float s=length(steer);
		if(s>max_force) steer*=max_force/s;

		return steer;
	}
};

const float Particle::max_speed=400;
const float Particle::max_force=30;
#endif