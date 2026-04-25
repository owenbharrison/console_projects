#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

//for rand
#include <cstdlib>

//for time
#include <ctime>

#include "shape.h"

#include "cmn/utils.h"

#include <list>

using cmn::float3;
using cmn::float2;

//y p => x y z
//0 0 => 1 0 0
static float3 polarToCartesian(float yaw, float pitch) {
	return {
		std::cos(yaw)*std::cos(pitch),
		std::sin(pitch),
		std::sin(yaw)*std::cos(pitch)
	};
}

struct Raymarching3DUI : public cmn::ConsoleEngine {
	float3 cam_pos{0, 0, 8};
	float cam_yaw=-cmn::Pi/2, cam_pitch=0;
	float3 cam_dir;
	float cam_fov=cmn::Pi/2;

	std::list<Shape*> shapes;

	bool raster_glow=true;

	bool user_create() override {
		app_title="Raymarching3D";

		shapes.push_back(new Sphere({0, 0, 1}, {-6, 0, 0}, 1));

		shapes.push_back(new BoxFrame({0, 1, 1}, {-3, 0, 0}, {1, 1, 1}, .1f));

		shapes.push_back(new Triangle({0, 1, 0}, {0, 1, 0}, {-1, -1, 0}, {1, -1, 0}));

		shapes.push_back(new Torus({1, 1, 0}, {3, 0, 0}, .75f, .25f));

		shapes.push_back(new Octahedron({1, 0, 0}, {6, 0, 0}, 1));

		return true;
	}

	void user_destroy() override {
		for(auto& s:shapes) delete s;
	}

	void handleCameraLooking(float dt) {
		//look left/right
		if(GetKey(SAPP_KEYCODE_LEFT).held) cam_yaw-=dt;
		if(GetKey(SAPP_KEYCODE_RIGHT).held) cam_yaw+=dt;

		//look up/down
		if(GetKey(SAPP_KEYCODE_UP).held) cam_pitch+=dt;
		if(GetKey(SAPP_KEYCODE_DOWN).held) cam_pitch-=dt;

		cam_pitch=std::clamp(cam_pitch, .001f-cmn::Pi/2, cmn::Pi/2-.001f);
	}

	void handleCameraMovement(float dt) {
		const float speed=4.67f*dt;
		//up/down
		if(GetKey(SAPP_KEYCODE_SPACE).held) cam_pos.y+=speed;
		if(GetKey(SAPP_KEYCODE_LEFT_SHIFT).held) cam_pos.y-=speed;

		//forward/back
		float3 fb_dir=normalize(float3(cam_dir.x, 0, cam_dir.z));
		if(GetKey(SAPP_KEYCODE_W).held) cam_pos+=speed*fb_dir;
		if(GetKey(SAPP_KEYCODE_S).held) cam_pos-=.6f*speed*fb_dir;

		//left/right
		float3 lr_dir=normalize(float3(-cam_dir.z, 0, cam_dir.x));
		if(GetKey(SAPP_KEYCODE_A).held) cam_pos-=.8f*speed*lr_dir;
		if(GetKey(SAPP_KEYCODE_D).held) cam_pos+=.8f*speed*lr_dir;
	}

	bool user_update(float dt) override {
		handleCameraLooking(dt);
		
		cam_dir=polarToCartesian(cam_yaw, cam_pitch);

		handleCameraMovement(dt);

		if(GetKey(SAPP_KEYCODE_ESCAPE).pressed) return false;

		if(GetKey(SAPP_KEYCODE_G).pressed) raster_glow^=true;

		return true;
	}

	bool user_raster() override {
		rst.clear({' ', 0, 0, 0});

		//https://en.wikipedia.org/wiki/Ray_tracing_(graphics)
		//where to look?
		float3 target=cam_pos+cam_dir;
		float3 v(0, 1, 0);//up
		float3 t=target-cam_pos;
		float3 b=cross(t, v);
		
		//coordinate system
		float3 tn=normalize(t);//fwd
		float3 bn=normalize(b);//rgt
		float3 vn=cross(tn, bn);//up

		//viewport sizes
		float gx=std::tan(cam_fov/2);
		float gy=gx*(rst.getHeight()-1)/(rst.getWidth()-1);

		//next pixel shift vectors
		float3 qx=2*gx/(rst.getWidth()-1)*bn;
		float3 qy=2*gy/(rst.getHeight()-1)*vn;

		//bottom left pixel center
		float3 p00=tn-gx*bn-gy*vn;
		for(int i=0; i<rst.getWidth(); i++) {
			for(int j=0; j<rst.getHeight(); j++) {
				float3 pij=p00+i*qx+j*qy;
				float3 rij=normalize(pij);

				float total_dist=0;
				float close_dist=INFINITY;
				float3 col;
				bool hit=false;
				while(total_dist<100) {
					float3 check_pt=cam_pos+total_dist*rij;
					float scene_dist=INFINITY;
					for(const auto& s:shapes) {
						float sd=s->signedDist(check_pt);
						if(sd<scene_dist) {
							col=s->col;
							scene_dist=sd;
						}
					}
					//save for glow
					if(scene_dist<close_dist) close_dist=scene_dist;
					//if hit something, exit
					if(scene_dist<1e-6f) {
						hit=true;
						break;
					}
					//else, march
					total_dist+=scene_dist;
				}

				unsigned char red=255*col.x;
				unsigned char green=255*col.y;
				unsigned char blue=255*col.z;
				if(hit) rst.draw(i, j, {' ', 0, 0, 0, red, green, blue});
				else if(raster_glow) {
					const float thresh=.25f;
					if(close_dist<thresh) {
						float pct=1-close_dist/thresh;
						int ix=std::clamp(int(8*pct), 0, 7);
						rst.draw(i, j, {" .,~=#&@"[ix], 255, 255, 255});
					}
				}
			}
		}
		return true;
	}
};

CMN_ENGINE_LAUNCH(Raymarching3DUI, 960, 540)