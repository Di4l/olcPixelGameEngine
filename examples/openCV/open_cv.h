//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
#include "olcPixelGameEngine.h"
#include "openCV/pgexOpenCV.h"
//-----------------------------------------------------------------------------

// Override base class with your custom functionality
class olcOpenCv : public olc::PixelGameEngine
{
public:
	olcOpenCv();
	virtual ~olcOpenCv();

	olc::Decal*    dclLogo;
	olc::CVSprite  olcLogo;
	olc::CVSprite* cvSprite = nullptr;

	uint8_t fadeIn(float dt, float time = 0.0);
	uint8_t fadeOut(float dt, float time = 0.0);

public:
	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
};
//-----------------------------------------------------------------------------