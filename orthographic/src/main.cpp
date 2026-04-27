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
	float d_xz=std::sqrt(p.x*p.x+p.z*p.z);
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

struct Orthographic : public cmn::ConsoleEngine {
	float3 cam_pos{2, 2, 3};
	float cam_yaw=0, cam_pitch=0;
	float3 cam_dir;
	
	float3 sun_pos{3.473f, 9.139f, 4.365f};

	std::vector<float3> vertexes;
	std::vector<Triangle> tris;

	bool user_create() override {
		app_title="Orthographic";

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
		
		if(GetKey(SAPP_KEYCODE_L).pressed) sun_pos=cam_pos;

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
			float3 t=project(v, cam_pos, rgt, up, fwd);
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
			const auto& ta=transformed_verts[t.a];
			const auto& tb=transformed_verts[t.b];
			const auto& tc=transformed_verts[t.c];
			if(ta.z<0||tb.z<0||tc.z<0) continue;
			
			//facing away from camera
			const auto& a=vertexes[t.a];
			const auto& b=vertexes[t.b];
			const auto& c=vertexes[t.c];
			float3 ab=b-a, ac=c-a;
			float3 norm=normalize(cross(ab, ac));
			float3 avg=(a+b+c)/3;
			if(dot(cam_pos-avg, norm)<0) continue;

			//dot product lighting
			float3 sun_dir=normalize(sun_pos-avg);
			float lum=std::clamp(dot(sun_dir, norm), 0.f, 1.f);
			int ix=std::clamp(int(10*lum), 0, 9);
			cmn::Glyph glyph{" .:-=+*#%@"[ix], 255, 255, 255};
			rst.fill_triangle_f(ta.x, ta.y, tb.x, tb.y, tc.x, tc.y, glyph);
		}

		return true;
	}
};

CMN_ENGINE_LAUNCH(Orthographic, 600, 600)