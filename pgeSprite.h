#pragma once

#include "pgeTypes.h"

namespace olc
{
	// O------------------------------------------------------------------------------O
	// | olc::Sprite - An image represented by a 2D array of olc::Pixel               |
	// O------------------------------------------------------------------------------O
	class Sprite
	{
	public:
		Sprite();
		Sprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		Sprite(int32_t w, int32_t h);
		~Sprite();

	public:
		olc::rcode LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode SaveToPGESprFile(const std::string& sImageFile);

	public:
		int32_t width = 0;
		int32_t height = 0;
		enum Mode { NORMAL, PERIODIC };
		enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };

	public:
		void SetSampleMode(olc::Sprite::Mode mode = olc::Sprite::Mode::NORMAL);
		Pixel GetPixel(int32_t x, int32_t y);
		bool  SetPixel(int32_t x, int32_t y, Pixel p);
		Pixel GetPixel(const olc::vi2d& a);
		bool  SetPixel(const olc::vi2d& a, Pixel p);
		Pixel Sample(float x, float y);
		Pixel SampleBL(float u, float v);
		Pixel* GetData();
		Pixel* pColData = nullptr;
		Mode modeSample = Mode::NORMAL;
	};
}