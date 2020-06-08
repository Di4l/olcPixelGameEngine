//-----------------------------------------------------------------------------
#include "open_cv.h"
//-----------------------------------------------------------------------------

// Override base class with your custom functionality
olcOpenCv::olcOpenCv() : dclLogo(nullptr), olcLogo()
{
	sAppName = "Retro Pop-Up Menu";
}
//-----------------------------------------------------------------------------

olcOpenCv::~olcOpenCv()
{
	if (cvSprite)
		delete cvSprite;
	cvSprite = nullptr;

	if (dclLogo)
		delete dclLogo;
	dclLogo = nullptr;
	//if(olcLogo)
	//	delete olcLogo;
	//olcLogo = nullptr;
}
//-----------------------------------------------------------------------------

uint8_t olcOpenCv::fadeIn(float dt, float time)
{
	return 0xff - fadeOut(dt, time);
}
//-----------------------------------------------------------------------------

uint8_t olcOpenCv::fadeOut(float dt, float time)
{
	static float stop = 0.0;
	static float accum = 0.0;

	if (time > 0)
	{
		stop = time;
		accum = 0.0;
	}

	return accum > stop ? 0x00 : uint8_t(float((stop - accum) * 0xff) / stop);
}
//-----------------------------------------------------------------------------

bool olcOpenCv::OnUserCreate()
{
	//olcLogo = new olc::CVSprite("E:/proyectos/olcPixelGameEngine/resources/olcPGE.png");
	olcLogo.LoadFromFile("E:/proyectos/olcPixelGameEngine/resources/olcPGE.png");
	dclLogo = new olc::Decal(&olcLogo);

	return true;
}
//-----------------------------------------------------------------------------

bool olcOpenCv::OnUserUpdate(float fElapsedTime)
{
	Clear(olc::BLACK);
	DrawDecal({ 0, 0 }, dclLogo, { 1,1 }, { 0xff, 0xd0, 0x50, 220 });

	//if (cvSprite)
	//	DrawSprite({ 0,0 }, cvSprite);

	return true;
}
//-----------------------------------------------------------------------------