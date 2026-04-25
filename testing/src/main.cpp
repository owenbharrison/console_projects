#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

//for swap
#include <algorithm>
//for sqrt
#include <cmath>
//for time
#include <ctime>
//for rand
#include <cstdlib>
#include <list>

float randFloat(float a, float b) {
	static const float recip=1.f/RAND_MAX;
	float t=recip*std::rand();
	return a+t*(b-a);
}

struct Demo : public cmn::ConsoleEngine {
	float mouse_x, mouse_y;

	//triangle
	int tx1=0, ty1=0, tx2=0, ty2=0, tx3=0, ty3=0;

	//circle
	int cx=0, cy=0, cr=0;

	//rect
	int rx1=0, ry1=0, rx2=0, ry2=0;

	//inversion
	bool invert=false;
	int ix=0, iy=0;

	struct particle_t { float x, y, vx, vy, span, age=0; };
	std::list<particle_t> particles;

	bool show_info=true;

	bool user_create() override {
		app_title="Console Engine Demo";

		std::srand(std::time(0));

		//randomize shapes
		int* x_ref[]{&tx1, &tx2, &tx3, &cx, &rx1, &rx2};
		int* y_ref[]{&ty1, &ty2, &ty3, &cy, &ry1, &ry2};
		int m=5;
		for(auto& x:x_ref) *x=randFloat(m, rst.getWidth()-m-1);
		for(auto& y:y_ref) *y=randFloat(m, rst.getHeight()-m-1);
		cr=randFloat(7, 15);

		return true;
	}

	void handleUserInput() {
		mouse_x=GetMouseX(), mouse_y=GetMouseY();

		//triangle placement
		if(GetKey(SAPP_KEYCODE_1).held) tx1=mouse_x, ty1=mouse_y;
		if(GetKey(SAPP_KEYCODE_2).held) tx2=mouse_x, ty2=mouse_y;
		if(GetKey(SAPP_KEYCODE_3).held) tx3=mouse_x, ty3=mouse_y;

		//circle placement
		const auto circle_action=GetKey(SAPP_KEYCODE_C);
		if(circle_action.pressed) cx=mouse_x, cy=mouse_y;
		if(circle_action.held) {
			int dx=mouse_x-cx, dy=mouse_y-cy;
			cr=std::sqrt(dx*dx+dy*dy);
		}

		//rectangle placement
		const auto rect_action=GetKey(SAPP_KEYCODE_R);
		if(rect_action.pressed) rx1=mouse_x, ry1=mouse_y;
		if(rect_action.held) rx2=mouse_x, ry2=mouse_y;

		//particle placement
		if(GetMouse(SAPP_MOUSEBUTTON_LEFT).held) {
			int num=1+std::rand()%3;
			for(int i=0; i<num; i++) {
				float x=mouse_x+randFloat(-1.5f, 1.5f);
				float y=mouse_y+randFloat(-1.5f, 1.5f);
				float vx=randFloat(-3.5f, 3.5f);
				float vy=randFloat(-3.5f, 3.5f);
				float die=randFloat(1.5f, 2.5f);
				particles.push_back({x, y, vx, vy, die});
			}
		}

		const auto invert_action=GetMouse(SAPP_MOUSEBUTTON_RIGHT);
		if(invert_action.pressed) invert=true, ix=mouse_x, iy=mouse_y;
		if(invert_action.released) invert=false;

		//toggle info menu
		if(GetKey(SAPP_KEYCODE_I).pressed) show_info^=true;
	}

	bool user_update(float dt) override {
		handleUserInput();

		//remove if too old, integrate otherwise
		for(auto it=particles.begin(); it!=particles.end();) {
			auto& p=*it;
			p.age+=dt;
			if(p.age>p.span) it=particles.erase(it);
			else p.x+=p.vx*dt, p.y+=p.vy*dt, it++;
		}

		return true;
	}

	void rasterCheckerboard(const cmn::Glyph& g) {
		const int sz=8;
		int num_x=1+rst.getWidth()/sz;
		int num_y=1+rst.getHeight()/sz;
		for(int i=0; i<num_x; i++) {
			for(int j=0; j<num_y; j++) {
				if((i+j)%2) continue;

				rst.fill_rect(sz*i, sz*j, sz, sz, g);
			}
		}
	}

	void rasterYellowRectangle() {
		int sx=rx1, sy=ry1;
		int ex=rx2, ey=ry2;
		if(sx>ex) std::swap(sx, ex);
		if(sy>ey) std::swap(sy, ey);
		int w=ex-sx, h=ey-sy;
		rst.fill_rect(sx, sy, w, h, {'r', 255, 255, 0});
		rst.draw_rect(sx, sy, w, h, {'R', 127, 127, 0});
	}

	//ascii flame gradient
	void rasterParticles() {
		unsigned char white[3]{255, 255, 255};
		unsigned char yellow[3]{255, 255, 0};
		unsigned char red[3]{255, 0, 0};

		for(const auto& p:particles) {
			float rel_age=p.age/p.span;
			char c=".:-=+*#%@"[int(9*(1-rel_age))];

			float t;
			unsigned char* col1, * col2;
			if(rel_age<.5f) t=2*rel_age, col1=white, col2=yellow;
			else t=2*(rel_age-.5f), col1=yellow, col2=red;
			unsigned char r=col1[0]+t*(col2[0]-col1[0]);
			unsigned char g=col1[1]+t*(col2[1]-col1[1]);
			unsigned char b=col1[2]+t*(col2[2]-col1[2]);
			rst.draw_f(p.x, p.y, {c, r, g, b});
		}
	}

	//invert glyph colors under selection
	void rasterInvert() {
		int sx=ix, sy=iy;
		int ex=mouse_x, ey=mouse_y;
		if(sx>ex) std::swap(sx, ex);
		if(sy>ey) std::swap(sy, ey);
		for(int i=sx; i<=ex; i++) {
			for(int j=sy; j<=ey; j++) {
				auto& g=rst.glyphs[rst.ix(i, j)];
				g.c=255-g.c;
				g.r=255-g.r, g.g=255-g.g, g.b=255-g.b;
				g.br=255-g.br, g.bg=255-g.bg, g.bb=255-g.bb;
			}
		}
	}

	void rasterInfo() {
		rst.fill_rect(0, 0, 21, 8, {' ', 0, 0, 0});
		cmn::Glyph white{' ', 255, 255, 255};
		rst.draw_string(0, 0, "terminal demo info", white);
		rst.draw_string(0, 2, "LMB: spawn particles", white);
		rst.draw_string(0, 3, "RMB: invert selection", white);
		rst.draw_string(0, 4, "R: drag a rectangle", white);
		rst.draw_string(0, 5, "C: drag a circle", white);
		rst.draw_string(0, 6, "1/2/3: place triangle", white);
		rst.draw_string(0, 7, "I: toggle this menu", white);
	}

	bool user_raster() override {
		//blue checkered background
		rst.clear({'a', 72, 57, 239});
		rasterCheckerboard({'b', 67, 163, 179});

		rasterYellowRectangle();

		//green circle
		rst.fill_circle(cx, cy, cr, {'c', 0, 255, 0});
		rst.draw_circle(cx, cy, cr, {'C', 0, 127, 0});

		//red triangle
		rst.fill_triangle(tx1, ty1, tx2, ty2, tx3, ty3, {'t', 255, 0, 0});
		rst.draw_triangle(tx1, ty1, tx2, ty2, tx3, ty3, {'T', 127, 0, 0});
		rst.draw(tx1, ty1, {'1', 127, 0, 0});
		rst.draw(tx2, ty2, {'2', 127, 0, 0});
		rst.draw(tx3, ty3, {'3', 127, 0, 0});

		rasterParticles();

		if(show_info) rasterInfo();

		if(invert) rasterInvert();

		return true;
	}
};

CMN_ENGINE_LAUNCH(Demo, 640, 480)