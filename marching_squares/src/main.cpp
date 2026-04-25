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

#include "cmn/float2.h"
using cmn::float2;

cmn::float2 min(const cmn::float2& a, const cmn::float2& b) {
	return {a.x<b.x?a.x:b.x, a.y<b.y?a.y:b.y};
}

cmn::float2 max(const cmn::float2& a, const cmn::float2& b) {
	return {a.x>b.x?a.x:b.x, a.y>b.y?a.y:b.y};
}

cmn::float2 mix(const cmn::float2& a, const cmn::float2& b, float t) {
	return a+t*(b-a);
}

//clever default param placement:
//random()=0-1
//random(a)=0-a
//random(a, b)=a-b
float randFloat(float b=1, float a=0) {
	static const float rand_max=RAND_MAX;
	float t=std::rand()/rand_max;
	return a+t*(b-a);
}

float clamp(float v, float a, float b) {
	return v<a?a:v>b?b:v;
}

float sign(float v) {
	return v==0?0:v>0?1:-1;
}

float invLerp(float v, float a, float b) {
	return (v-a)/(b-a);
}

//https://www.shadertoy.com/view/llVyWW
float sdPentagon(float2 p, float r) {
	const float a=.8090169f, b=.5877853f, c=.7265425f;
	p.x=std::abs(p.x);
	p-=2*float2(-a, b)*std::min(0.f, dot(float2(-a, b), p));
	p-=2*float2(a, b)*std::min(0.f, dot(float2(a, b), p));
	p-=float2(clamp(p.x, -r*c, r*c), r);
	return length(p)*sign(p.y);
}

//https://www.shadertoy.com/view/WtdBRS
float sdMoon(float2 p, float d, float ra, float rb) {
	p.y=std::abs(p.y);
	float a=(ra*ra-rb*rb+d*d)/(2*d);
	float b=std::sqrt(std::max(0.f, ra*ra-a*a));
	if(d*(p.x*b-p.y*a)>d*d*std::max(0.f, b-p.y)) {
		return length(p-float2(a, b));
	}
	return std::max((length(p)-ra), -(length(p-float2(d, 0))-rb));
}

//https://www.shadertoy.com/view/t3X3z4
float sdPentagram(float2 p, float r) {
	const float k1x=.8090169f;
	const float k2x=.3090169f;
	const float k1y=.5877853f;
	const float k2y=.9510565f;
	const float k1z=.7265425f;
	const float2 v1(k1x, -k1y);
	const float2 v2(-k1x, -k1y);
	const float2 v3(k2x, -k2y);
	p.x=std::abs(p.x);
	p-=2*std::max(0.f, dot(v1, p))*v1;
	p-=2*std::max(0.f, dot(v2, p))*v2;
	p.x=std::abs(p.x);
	p.y-=r;
	return length(p-v3*clamp(dot(p, v3), 0, k1z*r))*sign(p.y*v3.x-p.x*v3.y);
}

void stressGradient(float t, float& r, float& g, float& b) {
	static const float cols[5][3]{
		{0, 0, 1},//blue
		{0, 1, 1},//cyan
		{0, 1, 0},//green
		{1, 1, 0},//yellow
		{1, 0, 0}//red
	};
	static const int num=sizeof(cols)/sizeof(cols[0]);

	//clamp percent
	if(t<0) t=0;
	if(t>=1) t=.999f;

	//floor index, fract t
	float x=t*(num-1);
	int i=x;
	t=x-i;

	//lerp cols
	const auto& c1=cols[i];
	const auto& c2=cols[i+1];
	r=c1[0]+t*(c2[0]-c1[0]);
	g=c1[1]+t*(c2[1]-c1[1]);
	b=c1[2]+t*(c2[2]-c1[2]);
}

struct MarchingSquaresUI : public cmn::ConsoleEngine {
	const float min_cell_sz=1, max_cell_sz=5;
	float cell_sz=5;
	int width=0, height=0;
	float* val_grid=nullptr;
	float2* pos_grid=nullptr;
	float* col_grid=nullptr;
	int ix(int i, int j) { return i+width*j; }
	
	float2 mouse_pos;
	float2 a_pos, b_pos, c_pos;
	
	const int edge_table[16][4]{
		{-1, -1, -1, -1}, {0, 3, -1, -1},
		{0, 1, -1, -1}, {1, 3, -1, -1},
		{2, 3, -1, -1}, {0, 2, -1, -1},
		{0, 3, 1, 2}, {1, 2, -1, -1},
		{1, 2, -1, -1}, {0, 1, 2, 3},
		{0, 2, -1, -1}, {2, 3, -1, -1},
		{1, 3, -1, -1}, {0, 1, -1, -1},
		{0, 3, -1, -1}, {-1, -1, -1, -1}
	};

	bool user_create() override {
		app_title="Marching Squares";

		std::srand(std::time(0));

		cell_sz=randFloat(min_cell_sz, max_cell_sz);

		float2 res(rst.getWidth(), rst.getHeight());
		a_pos=float2(.25f, .25f)*res;
		b_pos=float2(.5f, .5f)*res;
		c_pos=float2(.75f, .75f)*res;

		return true;
	}

	void updateSizing() {
		int w=1+rst.getWidth()/cell_sz;
		int h=1+rst.getHeight()/cell_sz;
		if(w==width&&h==height) return;
		
		delete[] val_grid;
		delete[] pos_grid;
		delete[] col_grid;

		width=w, height=h;
		val_grid=new float[width*height];
		pos_grid=new float2[width*height];
		col_grid=new float[3*width*height];
	}

	//update values based on signed distance field
	void updateGrids() {
		float2 res(rst.getWidth(), rst.getHeight());
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				int k=ix(i, j);

				float2 p{cell_sz*i, cell_sz*j};
				pos_grid[k]=p;

				float a=sdPentagon(p-a_pos, 15);
				float b=sdMoon(p-b_pos, 6, 15, 10);
				float c=sdPentagram(p-c_pos, 15);
				val_grid[k]=std::min(std::min(a, b), c);
			}
		}

		//find absolute value range
		float max=1e-6f;
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				float v=std::abs(val_grid[ix(i, j)]);
				if(v>max) max=v;
			}
		}

		//color ramp
		for(int i=0; i<width; i++) {
			for(int j=0; j<height; j++) {
				int k=ix(i, j);

				stressGradient(
					std::abs(val_grid[k]/max),
					col_grid[3*k],
					col_grid[3*k+1],
					col_grid[3*k+2]
				);
			}
		}
	}

	bool user_update(float dt) override {
		//get mouse
		mouse_pos.x=GetMouseX();
		mouse_pos.y=GetMouseY();

		//move triangle
		if(GetKey(SAPP_KEYCODE_A).held) a_pos=mouse_pos;
		if(GetKey(SAPP_KEYCODE_B).held) b_pos=mouse_pos;
		if(GetKey(SAPP_KEYCODE_C).held) c_pos=mouse_pos;

		//change resolution?
		if(GetKey(SAPP_KEYCODE_UP).held) cell_sz*=(1+dt);
		if(GetKey(SAPP_KEYCODE_DOWN).held) cell_sz*=(1-dt);
		cell_sz=clamp(cell_sz, min_cell_sz, max_cell_sz);

		updateSizing();

		updateGrids();

		return true;
	}

	void rasterValues() {
		const float recip=1/cell_sz;
		for(int x=0; x<rst.getWidth(); x++) {
			for(int y=0; y<rst.getHeight(); y++) {
				int i=recip*x, j=recip*y;
				int k=ix(i, j);
				unsigned char r=255*col_grid[3*k];
				unsigned char g=255*col_grid[1+3*k];
				unsigned char b=255*col_grid[2+3*k];
				rst.draw(x, y, {' ', 0, 0, 0, r, g, b});
			}
		}
	}

	void rasterMarchedSquares(float thr, const cmn::Glyph& glf) {
		for(int i=0; i<width-1; i++) {
			for(int j=0; j<height-1; j++) {
				const auto& v0=val_grid[ix(i, j)];
				const auto& v1=val_grid[ix(i+1, j)];
				const auto& v2=val_grid[ix(i, j+1)];
				const auto& v3=val_grid[ix(i+1, j+1)];
				const auto& p0=pos_grid[ix(i, j)];
				const auto& p1=pos_grid[ix(i+1, j)];
				const auto& p2=pos_grid[ix(i, j+1)];
				const auto& p3=pos_grid[ix(i+1, j+1)];
				int state=8*(v3>thr)+4*(v2>thr)+2*(v1>thr)+(v0>thr);
				const auto& edge=edge_table[state];
				for(int k=0; k<4; k+=2) {
					if(edge[k]==-1) break;

					float2 p[2];
					for(int l=0; l<2; l++) {
						switch(edge[k+l]) {
							case 0: p[l]=mix(p0, p1, invLerp(thr, v0, v1)); break;
							case 1: p[l]=mix(p1, p3, invLerp(thr, v1, v3)); break;
							case 2: p[l]=mix(p3, p2, invLerp(thr, v3, v2)); break;
							case 3: p[l]=mix(p2, p0, invLerp(thr, v2, v0)); break;
						}
					}
					rst.draw_line_f(p[0].x, p[0].y, p[1].x, p[1].y, glf);
				}
			}
		}
	}

	bool user_raster() override {
		rst.clear({' ', 0, 0, 0, 0, 0, 0});

		rasterValues();

		rasterMarchedSquares(0, {'m', 255, 255, 255, 0, 0, 0});

		return true;
	}
};

CMN_ENGINE_LAUNCH(MarchingSquaresUI, 1280, 720)