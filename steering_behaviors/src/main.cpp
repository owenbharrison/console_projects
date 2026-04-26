#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include "cmn/console_engine.h"

#include "particle.h"

#include <list>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "cmn/utils.h"

//for time
#include <ctime>

using cmn::float2;

struct SteeringBehaviors : public cmn::ConsoleEngine {
	struct Image {
		int width=0, height=0;
		unsigned char* pixels=nullptr;
	};
	std::list<Image> images;
	
	std::list<Particle> particles;

	void placeParticles() {
		particles.clear();

		//choose random image
		auto img=images.begin();
		int ix_img=images.size()*cmn::randFloat();
		std::advance(img, ix_img%images.size());

		//get aspect ratios
		int w_img=img->width;
		int h_img=img->height;
		int w_rst=rst.getWidth();
		int h_rst=rst.getHeight();
		float asp_img=float(w_img)/h_img;
		float asp_rst=float(w_rst)/h_rst;
		float scl=asp_rst/asp_img;

		//for each pixel
		const float col_recip=1.f/255;
		for(int i=0; i<w_rst; i++) {
			for(int j=0; j<h_rst; j++) {
				//scale normalized uv
				float u11=2*(.5f+i)/w_rst-1;
				float v11=2*(.5f+j)/h_rst-1;
				if(scl>1) u11*=scl;
				else v11/=scl;

				//get pixel in img
				float u=.5f+.5f*u11;
				float v=.5f+.5f*v11;
				int i_img=u*w_img;
				int j_img=v*h_img;
				if(i_img<0||j_img<0||i_img>=w_img||j_img>=h_img) continue;
				
				//is transparent?
				int k_img=i_img+w_img*j_img;
				if(img->pixels[3+4*k_img]<127) continue;

				//place particle
				float x=cmn::randFloat(w_rst);
				float y=cmn::randFloat(h_rst);
				particles.push_back(Particle(
					{x, y},
					float2(i, j),
					col_recip*img->pixels[4*k_img],
					col_recip*img->pixels[1+4*k_img],
					col_recip*img->pixels[2+4*k_img]
				));
			}
		}
	}

	bool user_create() override {
		app_title="Steering Behaviors";

		std::srand(std::time(0));

		const std::list<std::string> filenames{
			"assets/2026.png",
			"assets/bike.png",
			"assets/cat.png",
			"assets/dog.png",
			"assets/house.png",
			"assets/truck.png"
		};
		for(const auto& f:filenames) {
			Image img;
			int comp;
			img.pixels=stbi_load(f.c_str(), &img.width, &img.height, &comp, 4);
			if(img.pixels) images.push_back(img);
		}

		placeParticles();

		return true;
	}

	void user_destroy() override {
		for(auto& i:images) delete[] i.pixels;
	}

	bool user_update(float dt) override {
		const float2 mouse_pos(GetMouseX(), GetMouseY());
		
		//randomize particles
		if(GetKey(SAPP_KEYCODE_R).pressed) {
			for(auto& p:particles) {
				p.pos.x=cmn::randFloat(rst.getWidth());
				p.pos.y=cmn::randFloat(rst.getHeight());
			}
		}

		if(GetKey(SAPP_KEYCODE_P).pressed) placeParticles();

		for(auto& p:particles) {
			//move towards target
			p.applyForce(20*p.getArrive(p.target));
			
			//if too close, flee mouse
			if(length(p.pos-mouse_pos)<7) {
				p.applyForce(50*p.getFlee(mouse_pos));
			}

			p.update(dt);
		}
		
		return true;
	}

	bool user_raster() override {
		rst.clear({' ', 0, 0, 0});
		
		//draw particles with ascii ramp
		for(const auto& p:particles) {
			float lum=(p.r+p.g+p.b)/3;
			int ix=std::clamp(int(7*lum), 0, 6);
			cmn::Glyph glyph;
			glyph.c=".,~=#&@"[ix];
			glyph.r=255*p.r;
			glyph.g=255*p.g;
			glyph.b=255*p.b;
			rst.draw_f(p.pos.x, p.pos.y, glyph);
		}

		return true;
	}
};

CMN_ENGINE_LAUNCH(SteeringBehaviors, 960, 540)