#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>

#include <cstdint>

#include <iostream>

int main() {
	//could be taken as argument
	std::string filename="assets/ibmbios_8x8.png";
	
	//try load image
	int width=0, height=0, comp=0;
	auto pixels=stbi_load(filename.c_str(), &width, &height, &comp, 4);
	if(!pixels) return 1;

	std::cout<<"\nLOADED IMG:\n\n";

	//print image
	for(int j=0; j<height; j++) {
		for(int i=0; i<width; i++) {
			int k=i+width*j;
			bool b=pixels[4*k]>127;
			std::cout<<(b?'#':' ');
		}
		std::cout<<'\n';
	}

	std::cout<<"\nPACKED INTS:\n\n";

	//bit bang the image into 64bit ints
	int num_packed=width*height/64;
	std::uint64_t* packed=new std::uint64_t[num_packed];
	std::memset(packed, 0, sizeof(std::uint64_t)*num_packed);
	for(int i=0; i<num_packed; i++) {
		auto& pack=packed[i];
		pack=0;
		for(int j=0; j<64; j++) {
			std::uint64_t b=pixels[4*(64*i+j)]>127;
			pack|=b<<j;
		}
	}
	
	//we can free the image now
	stbi_image_free(pixels);

	//print packed as 0-padded 16-wide hex
	for(int i=0; i<num_packed; i++) {
		std::printf("0x%016llx, ", packed[i]);
		if(i%6==5) std::cout<<'\n';
	}

	std::cout<<"\nRECONSTRUCTED IMG:\n\n";

	//test if image looks similar
	for(int j=0; j<height; j++){
		for(int i=0; i<width; i++) {
			int k=i+width*j;
			bool b=1&(packed[k/64]>>(k%64));
			std::cout<<(b?'#':' ');
		}
		std::cout<<'\n';
	}

	//we can free packed now
	delete[] packed;

	return 0;
}