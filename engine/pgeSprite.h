#pragma once

#include "pgeTypes.h"

namespace olc
{
	// O------------------------------------------------------------------------------O
	// | olc::Sprite - An image represented by a 2D array of olc::Pixel               |
	// O------------------------------------------------------------------------------O
	class Sprite : public ISprite
	{
	public:
		Sprite();
		Sprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		Sprite(int32_t w, int32_t h);
		virtual ~Sprite();

	public:
		olc::rcode LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode SaveToPGESprFile(const std::string& sImageFile);

	public:
		enum Mode { NORMAL, PERIODIC };
		enum Flip { NONE = 0, HORIZ = 1, VERT = 2 };

	public:
		void   SetSampleMode(olc::ISprite::Mode mode = olc::ISprite::Mode::NORMAL) override;
		Pixel  GetPixel(int32_t x, int32_t y);
		bool   SetPixel(int32_t x, int32_t y, Pixel p);
		Pixel  GetPixel(const olc::vi2d& a);
		bool   SetPixel(const olc::vi2d& a, Pixel p);
		Pixel  Sample(float x, float y);
		Pixel  SampleBL(float u, float v) override;
		Pixel* GetData() override;
		Pixel* pColData = nullptr;
	};
}