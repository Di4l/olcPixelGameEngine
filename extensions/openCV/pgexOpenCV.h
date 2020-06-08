//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
#include "olcPixelGameEngine.h"
#include "opencv2/core.hpp"
//-----------------------------------------------------------------------------
namespace olc
{
	//-------------------------------------------------------------------------

	class CVSprite : public Sprite
	{
	private:
		cv::Mat m_image;

		void updateSprite();

	protected:
		bool loadImage(const std::string& sImageFile);
		void scaleImage(float scale);

	public:
		CVSprite() : Sprite(), m_image() {}
		CVSprite(int32_t w, int32_t h) : Sprite(w, h), m_image(w, h, CV_8UC4) {}
		CVSprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		CVSprite(const std::string& sImageFile, int32_t w, int32_t h);
		CVSprite(const std::string& sImageFile, float scale);
		CVSprite(cv::Mat& image);
		virtual ~CVSprite();

		void resize(int32_t w, int32_t h);
	};
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

