//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
//#include "olcPixelGameEngine.h"
#include "pgeTypes.h"
#include "opencv2/core.hpp"
//-----------------------------------------------------------------------------
namespace olc
{
	//-------------------------------------------------------------------------

	class CVSprite : public ISprite
	{
	private:
		cv::Mat m_image;

		void updateSprite();

	protected:
		bool loadImage(const std::string& sImageFile);
		void scaleImage(float scale);

	public:
		CVSprite() : ISprite(), m_image() {}
		CVSprite(int32_t w, int32_t h) : ISprite(w, h), m_image(w, h, CV_8UC4) {}
		CVSprite(const std::string& sImageFile, olc::ResourcePack* pack = nullptr);
		CVSprite(const std::string& sImageFile, int32_t w, int32_t h);
		CVSprite(const std::string& sImageFile, float scale);
		CVSprite(cv::Mat& image);
		virtual ~CVSprite();

		void resize(int32_t w, int32_t h);

		virtual olc::rcode LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr) override { return olc::OK; }
		virtual olc::rcode LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack = nullptr) override { return olc::OK; }
		virtual olc::rcode SaveToPGESprFile(const std::string& sImageFile) override { return olc::OK; }

	};
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

