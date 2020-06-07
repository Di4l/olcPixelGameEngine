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

	olc::Sprite*   sprite   = nullptr;
	olc::CVSprite* cvSprite = nullptr;

public:
	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
};
//-----------------------------------------------------------------------------