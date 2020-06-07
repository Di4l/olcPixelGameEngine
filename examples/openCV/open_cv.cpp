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
	if (cvSprite)
		delete cvSprite;
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

	if (cvSprite)
		DrawSprite({ 0,0 }, cvSprite);

	return true;
}
//-----------------------------------------------------------------------------