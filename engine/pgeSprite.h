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
		Sprite() : ISprite() {}
		Sprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		Sprite(int32_t w, int32_t h) : ISprite(w, h) {}
		virtual ~Sprite() {}

	public:
		olc::rcode LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		olc::rcode SaveToPGESprFile(const std::string& sImageFile);

	public:
	};
}