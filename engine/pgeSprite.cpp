//-----------------------------------------------------------------------------
#include "pgeSprite.h"
//#include "pgeInternals.h"
//-----------------------------------------------------------------------------
using namespace olc;
//-----------------------------------------------------------------------------

Sprite::Sprite()
{
	pColData = nullptr; width = 0; height = 0;
}
//-----------------------------------------------------------------------------

Sprite::Sprite(const std::string& sImageFile, olc::ResourcePack* pack)
{
	LoadFromFile(sImageFile, pack);
}
//-----------------------------------------------------------------------------

Sprite::Sprite(int32_t w, int32_t h)
{
	if (pColData) delete[] pColData;
	width = w;		height = h;
	pColData = new Pixel[width * height];
	for (int32_t i = 0; i < width * height; i++)
		pColData[i] = Pixel();
}
//-----------------------------------------------------------------------------

Sprite::~Sprite()
{
	if (pColData) delete[] pColData;
}
//-----------------------------------------------------------------------------

olc::rcode Sprite::LoadFromPGESprFile(const std::string& sImageFile, olc::ResourcePack* pack)
{
	if (pColData) delete[] pColData;
	auto ReadData = [&](std::istream& is)
	{
		is.read((char*)&width, sizeof(int32_t));
		is.read((char*)&height, sizeof(int32_t));
		pColData = new Pixel[width * height];
		is.read((char*)pColData, (size_t)width * (size_t)height * sizeof(uint32_t));
	};

	// These are essentially Memory Surfaces represented by olc::Sprite
	// which load very fast, but are completely uncompressed
	if (pack == nullptr)
	{
		std::ifstream ifs;
		ifs.open(sImageFile, std::ifstream::binary);
		if (ifs.is_open())
		{
			ReadData(ifs);
			return olc::OK;
		}
		else
			return olc::FAIL;
	}
	else
	{
		ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
		std::istream is(&rb);
		ReadData(is);
		return olc::OK;
	}
	return olc::FAIL;
}
//-----------------------------------------------------------------------------

olc::rcode Sprite::SaveToPGESprFile(const std::string& sImageFile)
{
	if (pColData == nullptr) return olc::FAIL;

	std::ofstream ofs;
	ofs.open(sImageFile, std::ifstream::binary);
	if (ofs.is_open())
	{
		ofs.write((char*)&width, sizeof(int32_t));
		ofs.write((char*)&height, sizeof(int32_t));
		ofs.write((char*)pColData, (size_t)width * (size_t)height * sizeof(uint32_t));
		ofs.close();
		return olc::OK;
	}

	return olc::FAIL;
}
//-----------------------------------------------------------------------------

void Sprite::SetSampleMode(olc::ISprite::Mode mode)
{
	modeSample = mode;
}
//-----------------------------------------------------------------------------

Pixel Sprite::GetPixel(const olc::vi2d& a)
{
	return GetPixel(a.x, a.y);
}
//-----------------------------------------------------------------------------

bool Sprite::SetPixel(const olc::vi2d& a, Pixel p)
{
	return SetPixel(a.x, a.y, p);
}
//-----------------------------------------------------------------------------

Pixel Sprite::GetPixel(int32_t x, int32_t y)
{
	if (modeSample == olc::Sprite::Mode::NORMAL)
	{
		if (x >= 0 && x < width && y >= 0 && y < height)
			return pColData[y * width + x];
		else
			return Pixel(0, 0, 0, 0);
	}
	else
	{
		return pColData[abs(y % height) * width + abs(x % width)];
	}
}
//-----------------------------------------------------------------------------

bool Sprite::SetPixel(int32_t x, int32_t y, Pixel p)
{
	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		pColData[y * width + x] = p;
		return true;
	}
	else
		return false;
}
//-----------------------------------------------------------------------------

Pixel Sprite::Sample(float x, float y)
{
	int32_t sx = std::min((int32_t)((x * (float)width)), width - 1);
	int32_t sy = std::min((int32_t)((y * (float)height)), height - 1);
	return GetPixel(sx, sy);
}
//-----------------------------------------------------------------------------

Pixel Sprite::SampleBL(float u, float v)
{
	u = u * width - 0.5f;
	v = v * height - 0.5f;
	int x = (int)floor(u); // cast to int rounds toward zero, not downward
	int y = (int)floor(v); // Thanks @joshinils
	float u_ratio = u - x;
	float v_ratio = v - y;
	float u_opposite = 1 - u_ratio;
	float v_opposite = 1 - v_ratio;

	olc::Pixel p1 = GetPixel(std::max(x, 0), std::max(y, 0));
	olc::Pixel p2 = GetPixel(std::min(x + 1, (int)width - 1), std::max(y, 0));
	olc::Pixel p3 = GetPixel(std::max(x, 0), std::min(y + 1, (int)height - 1));
	olc::Pixel p4 = GetPixel(std::min(x + 1, (int)width - 1), std::min(y + 1, (int)height - 1));

	return olc::Pixel(
		(uint8_t)((p1.r * u_opposite + p2.r * u_ratio) * v_opposite + (p3.r * u_opposite + p4.r * u_ratio) * v_ratio),
		(uint8_t)((p1.g * u_opposite + p2.g * u_ratio) * v_opposite + (p3.g * u_opposite + p4.g * u_ratio) * v_ratio),
		(uint8_t)((p1.b * u_opposite + p2.b * u_ratio) * v_opposite + (p3.b * u_opposite + p4.b * u_ratio) * v_ratio));
}
//-----------------------------------------------------------------------------

Pixel* Sprite::GetData()
{
	return pColData;
}
//-----------------------------------------------------------------------------
