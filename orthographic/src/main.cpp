#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

#include "cmn/float3.h"

#include "cmn/utils.h"

#include <vector>

//starts at (1,0,0)
//positive yaw turns to right
//positive pitch turns up
static cmn::float3 polarToCartesian(float yaw, float pitch) {
	return {
		std::cos(yaw)*std::cos(pitch),
		std::sin(pitch),
		std::sin(yaw)*std::cos(pitch)
	};
}

//inverse of polarToCartesian
static void cartesianToPolar(
	const cmn::float3& p,
	float& yaw, float& pitch) {
	yaw=std::atan2(p.z, p.x);
	float d_xz=std::sqrt(p.x*p.x*p.z*p.z);
	pitch=std::atan2(p.y, d_xz);
}

//orthographic projection of point
// given camera's coordinate system
cmn::float3 project(
	const cmn::float3& p,
	const cmn::float3& e,
	const cmn::float3& x,
	const cmn::float3& y,
	const cmn::float3& z
) {
	return {
		dot(p-e, x),
		dot(p-e, y),
		dot(p-e, z)
	};
}

struct Triangle {
	int a, b, c;
};

using cmn::float3;

struct Voronoi : public cmn::ConsoleEngine {
	float3 cam_pos{2, 2, 2};
	float cam_yaw=0, cam_pitch=0;
	float3 cam_dir;
	
	std::vector<float3> vertexes;
	std::vector<Triangle> tris;

	bool user_create() override {
		app_title="Voronoi";

		//look at origin
		cartesianToPolar(-cam_pos, cam_yaw, cam_pitch);

		//cube
		vertexes={
			{-1, -1, -1},
			{1, -1, -1},
			{-1, 1, -1},
			{1, 1, -1},
			{-1, -1, 1},
			{1, -1, 1},
			{-1, 1, 1},
			{1, 1, 1}
		};
		tris={
			{0, 2, 1}, {1, 2, 3},
			{1, 3, 5}, {3, 7, 5},
			{4, 5, 6}, {5, 7, 6},
			{0, 4, 2}, {2, 4, 6},
			{2, 6, 3}, {3, 6, 7},
			{0, 1, 4}, {1, 5, 4}
		};

		return true;
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
		
		return true;
	}

	bool user_raster() override {
		//get camera coordinate system
		float3 pseudo_up(0, 1, 0);
		float3 fwd=cam_dir;
		float3 rgt=normalize(cross(fwd, pseudo_up));
		float3 up=cross(rgt, fwd);//will be unit

		//transform vertexes
		float x_ctr=.5f*rst.getWidth();
		float y_ctr=.5f*rst.getHeight();
		std::vector<float3> transformed_verts;
		for(const auto& v:vertexes) {
			float3 t=project(v, cam_pos, rgt, up, -fwd);
			//shift into screen
			t.x=x_ctr+20*t.x;
			t.y=y_ctr-20*t.y;
			transformed_verts.push_back(t);
		}

		//black background
		rst.clear({' ', 0, 0, 0});

		//draw all tri outlines
		const cmn::Glyph white{'#', 255, 255, 255};
		for(const auto& t:tris) {
			//behind camera?
			const auto& a=transformed_verts[t.a];
			const auto& b=transformed_verts[t.b];
			const auto& c=transformed_verts[t.c];
			if(a.z>0||b.z>0||c.z>0) continue;
			
			rst.draw_line_f(a.x, a.y, b.x, b.y, white);
			rst.draw_line_f(b.x, b.y, c.x, c.y, white);
			rst.draw_line_f(c.x, c.y, a.x, a.y, white);
		}

		return true;
	}
};

CMN_ENGINE_LAUNCH(Voronoi, 960, 540)