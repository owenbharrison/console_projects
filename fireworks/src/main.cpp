#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

#include "particle.h"

#include "cmn/utils.h"

//for time
#include <ctime>

#include <list>

using cmn::float2;

struct Fireworks : public cmn::ConsoleEngine {
	std::list<Particle> particles;
	float timer=0;

	const float2 gravity{0, 9.8f};

	bool user_create() override {
		app_title="Fireworks";

		std::srand(std::time(0));

		return true;
	}

	bool user_update(float dt) override {
		if(GetKey(SAPP_KEYCODE_ESCAPE).pressed) return false;

		timer-=dt;
		if(timer<0) {
			//spawn new seed at bottom of screen going up
			float2 pos(cmn::randFloat(rst.getWidth()), rst.getHeight());
			float vx=8*cmn::randFloat(-1, 1);
			float vy=-cmn::randFloat(30, 40);
			float lifespan=cmn::randFloat(2, 3);
			float r=cmn::randFloat();
			float g=cmn::randFloat();
			float b=cmn::randFloat();
			Particle p(pos, {vx, vy}, lifespan, r, g, b);
			p.seed=true;
			particles.push_back(p);

			//wait a random amount of time
			timer+=cmn::randFloat(.4f, 1.5f);
		}

		for(auto it=particles.begin(); it!=particles.end();) {
			it->applyForce(gravity);

			it->update(dt);

			if(it->isDead()) {
				if(it->seed) {
					int num=cmn::randInt(32, 56);
					for(int i=0; i<num; i++) {
						//emit in circle
						float dir=cmn::randFloat(2*cmn::Pi);
						float spd=cmn::randFloat(1, 14);
						float2 vel(spd*std::cosf(dir), spd*std::sinf(dir));
						float lifespan=cmn::randFloat(1.5f, 2.5f);
						//change color a little
						const float dc=.2f;
						float r=std::clamp(it->r+cmn::randFloat(dc), 0.f, 1.f);
						float g=std::clamp(it->g+cmn::randFloat(dc), 0.f, 1.f);
						float b=std::clamp(it->b+cmn::randFloat(dc), 0.f, 1.f);
						particles.push_back(Particle(it->pos, vel, lifespan, r, g, b));
					}
				}

				it=particles.erase(it);
			} else it++;
		}

		return true;
	}

	bool user_raster() override {
		rst.clear({' ', 0, 0, 0});
		
		for(const auto& p:particles) {
			 float t=1-p.age/p.lifespan;
			 int ix=std::clamp(int(10*t), 0, 9);
			 cmn::Glyph glyph;
			 glyph.c=" .:-=+*#%@"[ix];
			 glyph.r=255*p.r;
			 glyph.g=255*p.g;
			 glyph.b=255*p.b;
			 rst.draw_f(p.pos.x, p.pos.y, glyph);
		}

		return true;
	}
};

CMN_ENGINE_LAUNCH(Fireworks, 960, 540)