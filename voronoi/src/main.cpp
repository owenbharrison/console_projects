#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

#include "cmn/float2.h"

#include "cmn/utils.h"

struct Particle {
	cmn::float2 pos, vel;

	void update(float dt) {
		pos+=dt*vel;
	}

	void keepInBox(const cmn::float2& min, const cmn::float2& max) {
		//bounce only if moving toward wall
		if(pos.x<min.x) pos.x=min.x, vel.x*=vel.x<0?-1:1;
		if(pos.y<min.y) pos.y=min.y, vel.y*=vel.y<0?-1:1;
		if(pos.x>max.x) pos.x=max.x, vel.x*=vel.x>0?-1:1;
		if(pos.y>max.y) pos.y=max.y, vel.y*=vel.y>0?-1:1;
	}
};

float fract(float x) {
	return x-std::floor(x);
}

//https://www.shadertoy.com/view/4djSRW
void hash31(float p, float& x, float& y, float& z) {
	x=fract(.1031f*p);
	y=fract(.1030f*p);
	z=fract(.0973f*p);
	const float v=33.33f;
	float d=x*(v+y)+y*(v+z)+z*(v+x);
	x+=d, y+=d, z+=d;
	x=fract(z*(x+y));
	y=fract(y*(z+x));
	z=fract(x*(y+z));
}

using cmn::float2;

struct Voronoi : public cmn::ConsoleEngine {
	Particle particles[24];

	int width=0, height=0;
	int* grid=nullptr;
	int ix(int i, int j) const { return i+width*j; }

	bool use_manhattan=false;

	bool user_create() override {
		app_title="Voronoi";

		for(auto& p:particles) {
			//random placement
			p.pos.x=cmn::randFloat(rst.getWidth());
			p.pos.y=cmn::randFloat(rst.getHeight());
			//random velocity
			float dir=cmn::randFloat(2*cmn::Pi);
			float spd=cmn::randFloat(5, 12);
			p.vel.x=spd*std::cos(dir);
			p.vel.y=spd*std::sin(dir);
		}

		return true;
	}

	void updateSizing() {
		//avoid unnecessary allocations
		int w=rst.getWidth();
		int h=rst.getHeight();
		if(w!=width||h!=height) {
			delete[] grid;

			width=w, height=h;
			grid=new int[width*height];
		}
	}

	void updateGrid() {
		//for every pixel
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				float x=.5f+i, y=.5f+j;

				//find closest particle
				float record;
				int closest=-1;
				for(const auto& p:particles) {
					float dx=x-p.pos.x;
					float dy=y-p.pos.y;
					float d=use_manhattan?
						std::abs(dx)+std::abs(dy):
						std::sqrt(dx*dx+dy*dy);
					if(closest==-1||d<record) {
						record=d;
						closest=&p-particles;
					}
				}

				//store index of closest pt
				grid[ix(i, j)]=closest;
			}
		}
	}

	bool user_update(float dt) override {
		const float2 res(rst.getWidth(), rst.getHeight());
		for(auto& p:particles) {
			p.update(dt);

			p.keepInBox({0, 0}, res);
		}

		if(GetKey(SAPP_KEYCODE_ESCAPE).pressed) return false;

		updateSizing();

		updateGrid();

		return true;
	}

	bool user_raster() override {
		//show "cells"
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				float r, g, b;
				hash31(grid[ix(i, j)], r, g, b);

				cmn::Glyph col;
				col.c='a'+(i+j)%26;
				col.r=255*r, col.g=255*g, col.b=255*b;
				rst.draw(i, j, col);
			}
		}

		//edge detection
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				int curr=grid[ix(i, j)];

				//highlight differences between pixels
				bool diff=false;
				if(i>1) diff|=curr!=grid[ix(i-1, j)];//left
				if(j>1) diff|=curr!=grid[ix(i, j-1)];//up
				if(i<width-2) diff|=curr!=grid[ix(i+1, j)];//right
				if(j<height-2) diff|=curr!=grid[ix(i, j+1)];//down
				if(diff) rst.draw(i, j, {' ', 0, 0, 0, 255, 255, 255});
			}
		}

		return true;
	}
};

CMN_ENGINE_LAUNCH(Voronoi, 960, 540)