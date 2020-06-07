//-----------------------------------------------------------------------------
#include "open_cv.h"
//-----------------------------------------------------------------------------

// Override base class with your custom functionality
olcOpenCv::olcOpenCv()
{
	sAppName = "Retro Pop-Up Menu";
}
//-----------------------------------------------------------------------------

olcOpenCv::~olcOpenCv()
{
	if (sprite)
		delete sprite;
	if (cvSprite)
		delete cvSprite;

	sprite = nullptr;
	cvSprite = nullptr;
}
//-----------------------------------------------------------------------------

bool olcOpenCv::OnUserCreate()
{
	return true;
}
//-----------------------------------------------------------------------------

bool olcOpenCv::OnUserUpdate(float fElapsedTime)
{
	Clear(olc::BLACK);

	//if(sprite)
	//	DrawSprite({ 0,0 }, sprite);
	if (cvSprite)
		DrawSprite({ 0,0 }, cvSprite);

	return true;
}
//-----------------------------------------------------------------------------