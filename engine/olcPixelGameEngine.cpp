//-----------------------------------------------------------------------------
#include "olcPixelGameEngine.h"
#include "pgeDecal.h"
#include "pgeSprite.h"
//-----------------------------------------------------------------------------

// O--------------------------------------------------------------------------O
// | START OF OLC_PGE_APPLICATION                                             |
// O--------------------------------------------------------------------------O

// O--------------------------------------------------------------------------O
// | olcPixelGameEngine INTERFACE IMPLEMENTATION (CORE)                       |
// | Note: The core implementation is platform independent                    |
// O--------------------------------------------------------------------------O
namespace olc
{
	// O----------------------------------------------------------------------O
	// | olc::Pixel IMPLEMENTATION                                            |
	// O----------------------------------------------------------------------O
	Pixel::Pixel()
	{
		r = 0; g = 0; b = 0; a = nDefaultAlpha;
	}
	//-------------------------------------------------------------------------

	Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	{
		n = red | (green << 8) | (blue << 16) | (alpha << 24);
	} // Thanks jarekpelczar 
	//-------------------------------------------------------------------------

	Pixel::Pixel(uint32_t p)
	{
		n = p;
	}
	//-------------------------------------------------------------------------

	bool Pixel::operator==(const Pixel& p) const
	{
		return n == p.n;
	}
	//-------------------------------------------------------------------------

	bool Pixel::operator!=(const Pixel& p) const
	{
		return n != p.n;
	}
	//-------------------------------------------------------------------------

	Pixel PixelF(float red, float green, float blue, float alpha)
	{
		return Pixel(uint8_t(red * 255.0f), uint8_t(green * 255.0f), uint8_t(blue * 255.0f), uint8_t(alpha * 255.0f));
	}
	//-------------------------------------------------------------------------

	// O----------------------------------------------------------------------O
	// | olc::PixelGameEngine IMPLEMENTATION                                  |
	// O----------------------------------------------------------------------O
	PixelGameEngine::PixelGameEngine()
	{
		sAppName = "Undefined";
		olc::PGEX::pge = this;

		// Bring in relevant Platform & Rendering systems depending
		// on compiler parameters
		olc_ConfigureSystem();
	}
	//-------------------------------------------------------------------------

	PixelGameEngine::~PixelGameEngine()
	{}
	//-------------------------------------------------------------------------

	olc::rcode PixelGameEngine::Construct(int32_t screen_w, int32_t screen_h, int32_t pixel_w, int32_t pixel_h, bool full_screen, bool vsync)
	{
		vScreenSize = { screen_w, screen_h };
		vInvScreenSize = { 1.0f / float(screen_w), 1.0f / float(screen_h) };
		vPixelSize = { pixel_w, pixel_h };
		vWindowSize = vScreenSize * vPixelSize;
		bFullScreen = full_screen;
		bEnableVSYNC = vsync;
		vPixel = 2.0f / vScreenSize;

		if (vPixelSize.x <= 0 || vPixelSize.y <= 0 || vScreenSize.x <= 0 || vScreenSize.y <= 0)
			return olc::FAIL;


		return olc::OK;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetScreenSize(int w, int h)
	{
		vScreenSize = { w, h };
		for (auto& layer : vLayers)
		{
			delete layer.pDrawTarget; // Erase existing layer sprites
			layer.pDrawTarget = new Sprite(vScreenSize.x, vScreenSize.y);
			layer.bUpdate = true;
		}
		SetDrawTarget(nullptr);

		renderer->ClearBuffer(olc::BLACK, true);
		renderer->DisplayFrame();
		renderer->ClearBuffer(olc::BLACK, true);
		renderer->UpdateViewport(vViewPos, vViewSize);
	}
	//-------------------------------------------------------------------------

#if !defined(PGE_USE_CUSTOM_START)
	olc::rcode PixelGameEngine::Start()
	{
		if (platform->ApplicationStartUp() != olc::OK) return olc::FAIL;

		// Construct the window
		if (platform->CreateWindowPane({ 30,30 }, vWindowSize, bFullScreen) != olc::OK) return olc::FAIL;
		olc_UpdateWindowSize(vWindowSize.x, vWindowSize.y);

		// Start the thread
		bAtomActive = true;
		std::thread t = std::thread(&PixelGameEngine::EngineThread, this);

		// Some implementations may form an event loop here
		platform->StartSystemEventLoop();

		// Wait for thread to be exited
		t.join();

		if (platform->ApplicationCleanUp() != olc::OK) return olc::FAIL;

		return olc::OK;
	}
#endif
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetDrawTarget(ISprite* target)
	{
		if (target)
		{
			pDrawTarget = target;
		}
		else
		{
			nTargetLayer = 0;
			pDrawTarget = vLayers[0].pDrawTarget;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetDrawTarget(uint8_t layer)
	{
		if (layer < vLayers.size())
		{
			pDrawTarget = vLayers[layer].pDrawTarget;
			vLayers[layer].bUpdate = true;
			nTargetLayer = layer;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::EnableLayer(uint8_t layer, bool b)
	{
		if (layer < vLayers.size()) vLayers[layer].bShow = b;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerOffset(uint8_t layer, const olc::vf2d& offset)
	{
		SetLayerOffset(layer, offset.x, offset.y);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerOffset(uint8_t layer, float x, float y)
	{
		if (layer < vLayers.size()) vLayers[layer].vOffset = { x, y };
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerScale(uint8_t layer, const olc::vf2d& scale)
	{
		SetLayerScale(layer, scale.x, scale.y);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerScale(uint8_t layer, float x, float y)
	{
		if (layer < vLayers.size()) vLayers[layer].vScale = { x, y };
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerTint(uint8_t layer, const olc::Pixel& tint)
	{
		if (layer < vLayers.size()) vLayers[layer].tint = tint;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetLayerCustomRenderFunction(uint8_t layer, std::function<void()> f)
	{
		if (layer < vLayers.size()) vLayers[layer].funcHook = f;
	}
	//-------------------------------------------------------------------------

	std::vector<LayerDesc>& PixelGameEngine::GetLayers()
	{
		return vLayers;
	}
	//-------------------------------------------------------------------------

	uint32_t PixelGameEngine::CreateLayer()
	{
		LayerDesc ld;
		ld.pDrawTarget = new olc::Sprite(vScreenSize.x, vScreenSize.y);
		ld.nResID = renderer->CreateTexture(vScreenSize.x, vScreenSize.y);
		renderer->UpdateTexture(ld.nResID, ld.pDrawTarget);
		vLayers.push_back(ld);
		return uint32_t(vLayers.size()) - 1;
	}
	//-------------------------------------------------------------------------

	ISprite* PixelGameEngine::GetDrawTarget()
	{
		return pDrawTarget;
	}
	//-------------------------------------------------------------------------

	int32_t PixelGameEngine::GetDrawTargetWidth()
	{
		if (pDrawTarget)
			return pDrawTarget->width;
		else
			return 0;
	}
	//-------------------------------------------------------------------------

	int32_t PixelGameEngine::GetDrawTargetHeight()
	{
		if (pDrawTarget)
			return pDrawTarget->height;
		else
			return 0;
	}
	//-------------------------------------------------------------------------

	uint32_t PixelGameEngine::GetFPS()
	{
		return nLastFPS;
	}
	//-------------------------------------------------------------------------

	bool PixelGameEngine::IsFocused()
	{
		return bHasInputFocus;
	}
	//-------------------------------------------------------------------------

	HWButton PixelGameEngine::GetKey(Key k)
	{
		return pKeyboardState[k];
	}
	//-------------------------------------------------------------------------

	HWButton PixelGameEngine::GetMouse(uint32_t b)
	{
		return pMouseState[b];
	}
	//-------------------------------------------------------------------------

	int32_t PixelGameEngine::GetMouseX()
	{
		return vMousePos.x;
	}
	//-------------------------------------------------------------------------

	int32_t PixelGameEngine::GetMouseY()
	{
		return vMousePos.y;
	}
	//-------------------------------------------------------------------------

	int32_t PixelGameEngine::GetMouseWheel()
	{
		return nMouseWheelDelta;
	}
	//-------------------------------------------------------------------------

	const int32_t PixelGameEngine::ScreenWidth()
	{
		return vScreenSize.x;
	}
	//-------------------------------------------------------------------------

	const int32_t PixelGameEngine::ScreenHeight()
	{
		return vScreenSize.y;
	}
	//-------------------------------------------------------------------------

	bool PixelGameEngine::Draw(const olc::vi2d& pos, Pixel p)
	{
		return Draw(pos.x, pos.y, p);
	}
	//-------------------------------------------------------------------------

	// This is it, the critical function that plots a pixel
	bool PixelGameEngine::Draw(int32_t x, int32_t y, Pixel p)
	{
		if (!pDrawTarget) return false;

		if (nPixelMode == Pixel::NORMAL)
		{
			return pDrawTarget->SetPixel(x, y, p);
		}

		if (nPixelMode == Pixel::MASK)
		{
			if (p.a == 255)
				return pDrawTarget->SetPixel(x, y, p);
		}

		if (nPixelMode == Pixel::ALPHA)
		{
			Pixel d = pDrawTarget->GetPixel(x, y);
			float a = (float)(p.a / 255.0f) * fBlendFactor;
			float c = 1.0f - a;
			float r = a * (float)p.r + c * (float)d.r;
			float g = a * (float)p.g + c * (float)d.g;
			float b = a * (float)p.b + c * (float)d.b;
			return pDrawTarget->SetPixel(x, y, Pixel((uint8_t)r, (uint8_t)g, (uint8_t)b/*, (uint8_t)(p.a * fBlendFactor)*/));
		}

		if (nPixelMode == Pixel::CUSTOM)
		{
			return pDrawTarget->SetPixel(x, y, funcPixelMode(x, y, p, pDrawTarget->GetPixel(x, y)));
		}

		return false;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetSubPixelOffset(float ox, float oy)
	{
		//vSubPixelOffset.x = ox * vPixel.x;
		//vSubPixelOffset.y = oy * vPixel.y;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawLine(const olc::vi2d& pos1, const olc::vi2d& pos2, Pixel p, uint32_t pattern)
	{
		DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, p, pattern);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Pixel p, uint32_t pattern)
	{
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1; dy = y2 - y1;

		auto rol = [&](void) { pattern = (pattern << 1) | (pattern >> 31); return pattern & 1; };

		// straight lines idea by gurkanctn
		if (dx == 0) // Line is vertical
		{
			if (y2 < y1) std::swap(y1, y2);
			for (y = y1; y <= y2; y++) if (rol()) Draw(x1, y, p);
			return;
		}

		if (dy == 0) // Line is horizontal
		{
			if (x2 < x1) std::swap(x1, x2);
			for (x = x1; x <= x2; x++) if (rol()) Draw(x, y1, p);
			return;
		}

		// Line is Funk-aye
		dx1 = abs(dx); dy1 = abs(dy);
		px = 2 * dy1 - dx1;	py = 2 * dx1 - dy1;
		if (dy1 <= dx1)
		{
			if (dx >= 0)
			{
				x = x1; y = y1; xe = x2;
			}
			else
			{
				x = x2; y = y2; xe = x1;
			}

			if (rol()) Draw(x, y, p);

			for (i = 0; x < xe; i++)
			{
				x = x + 1;
				if (px < 0)
					px = px + 2 * dy1;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) y = y + 1; else y = y - 1;
					px = px + 2 * (dy1 - dx1);
				}
				if (rol()) Draw(x, y, p);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = x1; y = y1; ye = y2;
			}
			else
			{
				x = x2; y = y2; ye = y1;
			}

			if (rol()) Draw(x, y, p);

			for (i = 0; y < ye; i++)
			{
				y = y + 1;
				if (py <= 0)
					py = py + 2 * dx1;
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) x = x + 1; else x = x - 1;
					py = py + 2 * (dx1 - dy1);
				}
				if (rol()) Draw(x, y, p);
			}
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawCircle(const olc::vi2d& pos, int32_t radius, Pixel p, uint8_t mask)
	{
		DrawCircle(pos.x, pos.y, radius, p, mask);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawCircle(int32_t x, int32_t y, int32_t radius, Pixel p, uint8_t mask)
	{
		int x0 = 0;
		int y0 = radius;
		int d = 3 - 2 * radius;
		if (!radius) return;

		while (y0 >= x0) // only formulate 1/8 of circle
		{
			if (mask & 0x01) Draw(x + x0, y - y0, p);
			if (mask & 0x02) Draw(x + y0, y - x0, p);
			if (mask & 0x04) Draw(x + y0, y + x0, p);
			if (mask & 0x08) Draw(x + x0, y + y0, p);
			if (mask & 0x10) Draw(x - x0, y + y0, p);
			if (mask & 0x20) Draw(x - y0, y + x0, p);
			if (mask & 0x40) Draw(x - y0, y - x0, p);
			if (mask & 0x80) Draw(x - x0, y - y0, p);
			if (d < 0) d += 4 * x0++ + 6;
			else d += 4 * (x0++ - y0--) + 10;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::FillCircle(const olc::vi2d& pos, int32_t radius, Pixel p)
	{
		FillCircle(pos.x, pos.y, radius, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::FillCircle(int32_t x, int32_t y, int32_t radius, Pixel p)
	{
		// Taken from wikipedia
		int x0 = 0;
		int y0 = radius;
		int d = 3 - 2 * radius;
		if (!radius) return;

		auto drawline = [&](int sx, int ex, int ny)
		{
			for (int i = sx; i <= ex; i++)
				Draw(i, ny, p);
		};

		while (y0 >= x0)
		{
			// Modified to draw scan-lines instead of edges
			drawline(x - x0, x + x0, y - y0);
			drawline(x - y0, x + y0, y - x0);
			drawline(x - x0, x + x0, y + y0);
			drawline(x - y0, x + y0, y + x0);
			if (d < 0) d += 4 * x0++ + 6;
			else d += 4 * (x0++ - y0--) + 10;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawRect(const olc::vi2d& pos, const olc::vi2d& size, Pixel p)
	{
		DrawRect(pos.x, pos.y, size.x, size.y, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p)
	{
		DrawLine(x, y, x + w, y, p);
		DrawLine(x + w, y, x + w, y + h, p);
		DrawLine(x + w, y + h, x, y + h, p);
		DrawLine(x, y + h, x, y, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::Clear(Pixel p)
	{
		int pixels = GetDrawTargetWidth() * GetDrawTargetHeight();
		Pixel* m = GetDrawTarget()->GetData();
		for (int i = 0; i < pixels; i++) m[i] = p;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::ClearBuffer(Pixel p, bool bDepth)
	{
		renderer->ClearBuffer(p, bDepth);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::FillRect(const olc::vi2d& pos, const olc::vi2d& size, Pixel p)
	{
		FillRect(pos.x, pos.y, size.x, size.y, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p)
	{
		int32_t x2 = x + w;
		int32_t y2 = y + h;

		if (x < 0) x = 0;
		if (x >= (int32_t)GetDrawTargetWidth()) x = (int32_t)GetDrawTargetWidth();
		if (y < 0) y = 0;
		if (y >= (int32_t)GetDrawTargetHeight()) y = (int32_t)GetDrawTargetHeight();

		if (x2 < 0) x2 = 0;
		if (x2 >= (int32_t)GetDrawTargetWidth()) x2 = (int32_t)GetDrawTargetWidth();
		if (y2 < 0) y2 = 0;
		if (y2 >= (int32_t)GetDrawTargetHeight()) y2 = (int32_t)GetDrawTargetHeight();

		for (int i = x; i < x2; i++)
			for (int j = y; j < y2; j++)
				Draw(i, j, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawTriangle(const olc::vi2d& pos1, const olc::vi2d& pos2, const olc::vi2d& pos3, Pixel p)
	{
		DrawTriangle(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p)
	{
		DrawLine(x1, y1, x2, y2, p);
		DrawLine(x2, y2, x3, y3, p);
		DrawLine(x3, y3, x1, y1, p);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::FillTriangle(const olc::vi2d& pos1, const olc::vi2d& pos2, const olc::vi2d& pos3, Pixel p)
	{
		FillTriangle(pos1.x, pos1.y, pos2.x, pos2.y, pos3.x, pos3.y, p);
	}
	//-------------------------------------------------------------------------

	// https://www.avrfreaks.net/sites/default/files/triangles.c
	void PixelGameEngine::FillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p)
	{
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, p); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		// Sort vertices
		if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
		if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3); }
		if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3); }

		t1x = t2x = x1; y = y1;   // Starting points
		dx1 = (int)(x2 - x1);
		if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y2 - y1);

		dx2 = (int)(x3 - x1);
		if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
		else signx2 = 1;
		dy2 = (int)(y3 - y1);

		if (dy1 > dx1) { std::swap(dx1, dy1); changed1 = true; }
		if (dy2 > dx2) { std::swap(dy2, dx2); changed2 = true; }

		e2 = (int)(dx2 >> 1);
		// Flat top, just process the second half
		if (y1 == y2) goto next;
		e1 = (int)(dx1 >> 1);

		for (int i = 0; i < dx1;) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) t1xp = signx1;//t1x += signx1;
					else          goto next1;
				}
				if (changed1) break;
				else t1x += signx1;
			}
			// Move line
		next1:
			// process second line until y value is about to change
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;//t2x += signx2;
					else          goto next2;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next2:
			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);    // Draw line from min to max points found on the y
										// Now increase y
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y == y2) break;

		}
	next:
		// Second half
		dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
		else signx1 = 1;
		dy1 = (int)(y3 - y2);
		t1x = x2;

		if (dy1 > dx1) {   // swap values
			std::swap(dy1, dx1);
			changed1 = true;
		}
		else changed1 = false;

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i <= dx1; i++) {
			t1xp = 0; t2xp = 0;
			if (t1x < t2x) { minx = t1x; maxx = t2x; }
			else { minx = t2x; maxx = t1x; }
			// process first line until y value is about to change
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) { t1xp = signx1; break; }//t1x += signx1;
					else          goto next3;
				}
				if (changed1) break;
				else   	   	  t1x += signx1;
				if (i < dx1) i++;
			}
		next3:
			// process second line until y value is about to change
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) t2xp = signx2;
					else          goto next4;
				}
				if (changed2)     break;
				else              t2x += signx2;
			}
		next4:

			if (minx > t1x) minx = t1x;
			if (minx > t2x) minx = t2x;
			if (maxx < t1x) maxx = t1x;
			if (maxx < t2x) maxx = t2x;
			drawline(minx, maxx, y);
			if (!changed1) t1x += signx1;
			t1x += t1xp;
			if (!changed2) t2x += signx2;
			t2x += t2xp;
			y += 1;
			if (y > y3) return;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawSprite(const olc::vi2d& pos, ISprite* sprite, uint32_t scale, uint8_t flip)
	{
		DrawSprite(pos.x, pos.y, sprite, scale, flip);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawSprite(int32_t x, int32_t y, ISprite* sprite, uint32_t scale, uint8_t flip)
	{
		if (sprite == nullptr)
			return;

		int32_t fxs = 0, fxm = 1, fx = 0;
		int32_t fys = 0, fym = 1, fy = 0;
		if (flip & olc::Sprite::Flip::HORIZ) { fxs = sprite->width - 1; fxm = -1; }
		if (flip & olc::Sprite::Flip::VERT) { fys = sprite->height - 1; fym = -1; }

		if (scale > 1)
		{
			fx = fxs;
			for (int32_t i = 0; i < sprite->width; i++, fx += fxm)
			{
				fy = fys;
				for (int32_t j = 0; j < sprite->height; j++, fy += fym)
					for (uint32_t is = 0; is < scale; is++)
						for (uint32_t js = 0; js < scale; js++)
							Draw(x + (i * scale) + is, y + (j * scale) + js, sprite->GetPixel(fx, fy));
			}
		}
		else
		{
			fx = fxs;
			for (int32_t i = 0; i < sprite->width; i++, fx += fxm)
			{
				fy = fys;
				for (int32_t j = 0; j < sprite->height; j++, fy += fym)
					Draw(x + i, y + j, sprite->GetPixel(fx, fy));
			}
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialSprite(const olc::vi2d& pos, ISprite* sprite, const olc::vi2d& sourcepos, const olc::vi2d& size, uint32_t scale, uint8_t flip)
	{
		DrawPartialSprite(pos.x, pos.y, sprite, sourcepos.x, sourcepos.y, size.x, size.y, scale, flip);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialSprite(int32_t x, int32_t y, ISprite* sprite, int32_t ox, int32_t oy, int32_t w, int32_t h, uint32_t scale, uint8_t flip)
	{
		if (sprite == nullptr)
			return;

		int32_t fxs = 0, fxm = 1, fx = 0;
		int32_t fys = 0, fym = 1, fy = 0;
		if (flip & olc::Sprite::Flip::HORIZ) { fxs = w - 1; fxm = -1; }
		if (flip & olc::Sprite::Flip::VERT) { fys = h - 1; fym = -1; }

		if (scale > 1)
		{
			fx = fxs;
			for (int32_t i = 0; i < w; i++, fx += fxm)
			{
				fy = fys;
				for (int32_t j = 0; j < h; j++, fy += fym)
					for (uint32_t is = 0; is < scale; is++)
						for (uint32_t js = 0; js < scale; js++)
							Draw(x + (i * scale) + is, y + (j * scale) + js, sprite->GetPixel(fx + ox, fy + oy));
			}
		}
		else
		{
			fx = fxs;
			for (int32_t i = 0; i < w; i++, fx += fxm)
			{
				fy = fys;
				for (int32_t j = 0; j < h; j++, fy += fym)
					Draw(x + i, y + j, sprite->GetPixel(fx + ox, fy + oy));
			}
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialDecal(const olc::vf2d& pos, olc::Decal* decal, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::vf2d& scale, const olc::Pixel& tint)
	{
		olc::vf2d vScreenSpacePos =
		{
			(pos.x * vInvScreenSize.x) * 2.0f - 1.0f,
			((pos.y * vInvScreenSize.y) * 2.0f - 1.0f) * -1.0f
		};

		olc::vf2d vScreenSpaceDim =
		{
			vScreenSpacePos.x + (2.0f * source_size.x * vInvScreenSize.x) * scale.x,
			vScreenSpacePos.y - (2.0f * source_size.y * vInvScreenSize.y) * scale.y
		};

		DecalInstance di; di.decal = decal;	di.tint = tint;

		di.pos[0] = { vScreenSpacePos.x, vScreenSpacePos.y };
		di.pos[1] = { vScreenSpacePos.x, vScreenSpaceDim.y };
		di.pos[2] = { vScreenSpaceDim.x, vScreenSpaceDim.y };
		di.pos[3] = { vScreenSpaceDim.x, vScreenSpacePos.y };

		olc::vf2d uvtl = source_pos * decal->vUVScale;
		olc::vf2d uvbr = uvtl + (source_size * decal->vUVScale);
		di.uv[0] = { uvtl.x, uvtl.y }; di.uv[1] = { uvtl.x, uvbr.y };
		di.uv[2] = { uvbr.x, uvbr.y }; di.uv[3] = { uvbr.x, uvtl.y };
		vLayers[nTargetLayer].vecDecalInstance.push_back(di);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawDecal(const olc::vf2d& pos, olc::Decal* decal, const olc::vf2d& scale, const olc::Pixel& tint)
	{
		olc::vf2d vScreenSpacePos =
		{
			(pos.x * vInvScreenSize.x) * 2.0f - 1.0f,
			((pos.y * vInvScreenSize.y) * 2.0f - 1.0f) * -1.0f
		};

		olc::vf2d vScreenSpaceDim =
		{
			vScreenSpacePos.x + (2.0f * (float(decal->sprite->width) * vInvScreenSize.x)) * scale.x,
			vScreenSpacePos.y - (2.0f * (float(decal->sprite->height) * vInvScreenSize.y)) * scale.y
		};

		DecalInstance di;
		di.decal = decal;
		di.tint = tint;
		di.pos[0] = { vScreenSpacePos.x, vScreenSpacePos.y };
		di.pos[1] = { vScreenSpacePos.x, vScreenSpaceDim.y };
		di.pos[2] = { vScreenSpaceDim.x, vScreenSpaceDim.y };
		di.pos[3] = { vScreenSpaceDim.x, vScreenSpacePos.y };
		vLayers[nTargetLayer].vecDecalInstance.push_back(di);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawRotatedDecal(const olc::vf2d& pos, olc::Decal* decal, const float fAngle, const olc::vf2d& center, const olc::vf2d& scale, const olc::Pixel& tint)
	{
		DecalInstance di;
		di.decal = decal;
		di.tint = tint;
		di.pos[0] = (olc::vf2d(0.0f, 0.0f) - center) * scale;
		di.pos[1] = (olc::vf2d(0.0f, float(decal->sprite->height)) - center) * scale;
		di.pos[2] = (olc::vf2d(float(decal->sprite->width), float(decal->sprite->height)) - center) * scale;
		di.pos[3] = (olc::vf2d(float(decal->sprite->width), 0.0f) - center) * scale;
		float c = cos(fAngle), s = sin(fAngle);
		for (int i = 0; i < 4; i++)
		{
			di.pos[i] = pos + olc::vf2d(di.pos[i].x * c - di.pos[i].y * s, di.pos[i].x * s + di.pos[i].y * c);
			di.pos[i] = di.pos[i] * vInvScreenSize * 2.0f - olc::vf2d(1.0f, 1.0f);
			di.pos[i].y *= -1.0f;
		}
		vLayers[nTargetLayer].vecDecalInstance.push_back(di);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialRotatedDecal(const olc::vf2d& pos, olc::Decal* decal, const float fAngle, const olc::vf2d& center, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::vf2d& scale, const olc::Pixel& tint)
	{
		DecalInstance di;
		di.decal = decal;
		di.tint = tint;
		di.pos[0] = (olc::vf2d(0.0f, 0.0f) - center) * scale;
		di.pos[1] = (olc::vf2d(0.0f, source_size.y) - center) * scale;
		di.pos[2] = (olc::vf2d(source_size.x, source_size.y) - center) * scale;
		di.pos[3] = (olc::vf2d(source_size.x, 0.0f) - center) * scale;
		float c = cos(fAngle), s = sin(fAngle);
		for (int i = 0; i < 4; i++)
		{
			di.pos[i] = pos + olc::vf2d(di.pos[i].x * c - di.pos[i].y * s, di.pos[i].x * s + di.pos[i].y * c);
			di.pos[i] = di.pos[i] * vInvScreenSize * 2.0f - olc::vf2d(1.0f, 1.0f);
			di.pos[i].y *= -1.0f;
		}

		olc::vf2d uvtl = source_pos * decal->vUVScale;
		olc::vf2d uvbr = uvtl + (source_size * decal->vUVScale);
		di.uv[0] = { uvtl.x, uvtl.y }; di.uv[1] = { uvtl.x, uvbr.y };
		di.uv[2] = { uvbr.x, uvbr.y }; di.uv[3] = { uvbr.x, uvtl.y };

		vLayers[nTargetLayer].vecDecalInstance.push_back(di);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialWarpedDecal(olc::Decal* decal, const olc::vf2d* pos, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint)
	{
		DecalInstance di;
		di.decal = decal;
		di.tint = tint;
		olc::vf2d center;
		float rd = ((pos[2].x - pos[0].x) * (pos[3].y - pos[1].y) - (pos[3].x - pos[1].x) * (pos[2].y - pos[0].y));
		if (rd != 0)
		{
			olc::vf2d uvtl = source_pos * decal->vUVScale;
			olc::vf2d uvbr = uvtl + (source_size * decal->vUVScale);
			di.uv[0] = { uvtl.x, uvtl.y }; di.uv[1] = { uvtl.x, uvbr.y };
			di.uv[2] = { uvbr.x, uvbr.y }; di.uv[3] = { uvbr.x, uvtl.y };

			rd = 1.0f / rd;
			float rn = ((pos[3].x - pos[1].x) * (pos[0].y - pos[1].y) - (pos[3].y - pos[1].y) * (pos[0].x - pos[1].x)) * rd;
			float sn = ((pos[2].x - pos[0].x) * (pos[0].y - pos[1].y) - (pos[2].y - pos[0].y) * (pos[0].x - pos[1].x)) * rd;
			if (!(rn < 0.f || rn > 1.f || sn < 0.f || sn > 1.f)) center = pos[0] + rn * (pos[2] - pos[0]);
			float d[4];	for (int i = 0; i < 4; i++)	d[i] = (pos[i] - center).mag();
			for (int i = 0; i < 4; i++)
			{
				float q = d[i] == 0.0f ? 1.0f : (d[i] + d[(i + 2) & 3]) / d[(i + 2) & 3];
				di.uv[i] *= q; di.w[i] *= q;
				di.pos[i] = { (pos[i].x * vInvScreenSize.x) * 2.0f - 1.0f, ((pos[i].y * vInvScreenSize.y) * 2.0f - 1.0f) * -1.0f };
			}
			vLayers[nTargetLayer].vecDecalInstance.push_back(di);
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawWarpedDecal(olc::Decal* decal, const olc::vf2d* pos, const olc::Pixel& tint)
	{
		// Thanks Nathan Reed, a brilliant article explaining whats going on here
		// http://www.reedbeta.com/blog/quadrilateral-interpolation-part-1/
		DecalInstance di;
		di.decal = decal;
		di.tint = tint;
		olc::vf2d center;
		float rd = ((pos[2].x - pos[0].x) * (pos[3].y - pos[1].y) - (pos[3].x - pos[1].x) * (pos[2].y - pos[0].y));
		if (rd != 0)
		{
			rd = 1.0f / rd;
			float rn = ((pos[3].x - pos[1].x) * (pos[0].y - pos[1].y) - (pos[3].y - pos[1].y) * (pos[0].x - pos[1].x)) * rd;
			float sn = ((pos[2].x - pos[0].x) * (pos[0].y - pos[1].y) - (pos[2].y - pos[0].y) * (pos[0].x - pos[1].x)) * rd;
			if (!(rn < 0.f || rn > 1.f || sn < 0.f || sn > 1.f)) center = pos[0] + rn * (pos[2] - pos[0]);
			float d[4];	for (int i = 0; i < 4; i++)	d[i] = (pos[i] - center).mag();
			for (int i = 0; i < 4; i++)
			{
				float q = d[i] == 0.0f ? 1.0f : (d[i] + d[(i + 2) & 3]) / d[(i + 2) & 3];
				di.uv[i] *= q; di.w[i] *= q;
				di.pos[i] = { (pos[i].x * vInvScreenSize.x) * 2.0f - 1.0f, ((pos[i].y * vInvScreenSize.y) * 2.0f - 1.0f) * -1.0f };
			}
			vLayers[nTargetLayer].vecDecalInstance.push_back(di);
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawWarpedDecal(olc::Decal* decal, const std::array<olc::vf2d, 4>& pos, const olc::Pixel& tint)
	{
		DrawWarpedDecal(decal, pos.data(), tint);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawWarpedDecal(olc::Decal* decal, const olc::vf2d(&pos)[4], const olc::Pixel& tint)
	{
		DrawWarpedDecal(decal, &pos[0], tint);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialWarpedDecal(olc::Decal* decal, const std::array<olc::vf2d, 4>& pos, const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint)
	{
		DrawPartialWarpedDecal(decal, pos.data(), source_pos, source_size, tint);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawPartialWarpedDecal(olc::Decal* decal, const olc::vf2d(&pos)[4], const olc::vf2d& source_pos, const olc::vf2d& source_size, const olc::Pixel& tint)
	{
		DrawPartialWarpedDecal(decal, &pos[0], source_pos, source_size, tint);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawStringDecal(const olc::vf2d& pos, const std::string& sText, const Pixel col, const olc::vf2d& scale)
	{
		olc::vf2d spos = { 0.0f, 0.0f };
		for (auto c : sText)
		{
			if (c == '\n')
			{
				spos.x = 0; spos.y += 8.0f * scale.y;
			}
			else
			{
				int32_t ox = (c - 32) % 16;
				int32_t oy = (c - 32) / 16;
				DrawPartialDecal(pos + spos, fontDecal, { float(ox) * 8.0f, float(oy) * 8.0f }, { 8.0f, 8.0f }, scale, col);
				spos.x += 8.0f * scale.x;
			}
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawString(const olc::vi2d& pos, const std::string& sText, Pixel col, uint32_t scale)
	{
		DrawString(pos.x, pos.y, sText, col, scale);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::DrawString(int32_t x, int32_t y, const std::string& sText, Pixel col, uint32_t scale)
	{
		int32_t sx = 0;
		int32_t sy = 0;
		Pixel::Mode m = nPixelMode;
		// Thanks @tucna, spotted bug with col.ALPHA :P
		if (col.a != 255)		SetPixelMode(Pixel::ALPHA);
		else					SetPixelMode(Pixel::MASK);
		for (auto c : sText)
		{
			if (c == '\n')
			{
				sx = 0; sy += 8 * scale;
			}
			else
			{
				int32_t ox = (c - 32) % 16;
				int32_t oy = (c - 32) / 16;

				if (scale > 1)
				{
					for (uint32_t i = 0; i < 8; i++)
						for (uint32_t j = 0; j < 8; j++)
							if (fontSprite->GetPixel(i + ox * 8, j + oy * 8).r > 0)
								for (uint32_t is = 0; is < scale; is++)
									for (uint32_t js = 0; js < scale; js++)
										Draw(x + sx + (i * scale) + is, y + sy + (j * scale) + js, col);
				}
				else
				{
					for (uint32_t i = 0; i < 8; i++)
						for (uint32_t j = 0; j < 8; j++)
							if (fontSprite->GetPixel(i + ox * 8, j + oy * 8).r > 0)
								Draw(x + sx + i, y + sy + j, col);
				}
				sx += 8 * scale;
			}
		}
		SetPixelMode(m);
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetPixelMode(Pixel::Mode m)
	{
		nPixelMode = m;
	}
	//-------------------------------------------------------------------------

	Pixel::Mode PixelGameEngine::GetPixelMode()
	{
		return nPixelMode;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetPixelMode(std::function<olc::Pixel(const int x, const int y, const olc::Pixel&, const olc::Pixel&)> pixelMode)
	{
		funcPixelMode = pixelMode;
		nPixelMode = Pixel::Mode::CUSTOM;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::SetPixelBlend(float fBlend)
	{
		fBlendFactor = fBlend;
		if (fBlendFactor < 0.0f) fBlendFactor = 0.0f;
		if (fBlendFactor > 1.0f) fBlendFactor = 1.0f;
	}
	//-------------------------------------------------------------------------

	// User must override these functions as required. I have not made
	// them abstract because I do need a default behaviour to occur if
	// they are not overwritten

	bool PixelGameEngine::OnUserCreate()
	{
		return false;
	}
	//-------------------------------------------------------------------------

	bool PixelGameEngine::OnUserUpdate(float fElapsedTime)
	{
		UNUSED(fElapsedTime);  return false;
	}
	//-------------------------------------------------------------------------

	bool PixelGameEngine::OnUserDestroy()
	{
		return true;
	}
	//-------------------------------------------------------------------------
	//////////////////////////////////////////////////////////////////

	void PixelGameEngine::olc_UpdateViewport()
	{
		int32_t ww = vScreenSize.x * vPixelSize.x;
		int32_t wh = vScreenSize.y * vPixelSize.y;
		float wasp = (float)ww / (float)wh;

		vViewSize.x = (int32_t)vWindowSize.x;
		vViewSize.y = (int32_t)((float)vViewSize.x / wasp);

		if (vViewSize.y > vWindowSize.y)
		{
			vViewSize.y = vWindowSize.y;
			vViewSize.x = (int32_t)((float)vViewSize.y * wasp);
		}

		vViewPos = (vWindowSize - vViewSize) / 2;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateWindowSize(int32_t x, int32_t y)
	{
		vWindowSize = { x, y };
		olc_UpdateViewport();
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateMouseWheel(int32_t delta)
	{
		nMouseWheelDeltaCache += delta;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateMouse(int32_t x, int32_t y)
	{
		// Mouse coords come in screen space
		// But leave in pixel space

		// Full Screen mode may have a weird viewport we must clamp to
		x -= vViewPos.x;
		y -= vViewPos.y;
		vMousePosCache.x = (int32_t)(((float)x / (float)(vWindowSize.x - (vViewPos.x * 2)) * (float)vScreenSize.x));
		vMousePosCache.y = (int32_t)(((float)y / (float)(vWindowSize.y - (vViewPos.y * 2)) * (float)vScreenSize.y));
		if (vMousePosCache.x >= (int32_t)vScreenSize.x)	vMousePosCache.x = vScreenSize.x - 1;
		if (vMousePosCache.y >= (int32_t)vScreenSize.y)	vMousePosCache.y = vScreenSize.y - 1;
		if (vMousePosCache.x < 0) vMousePosCache.x = 0;
		if (vMousePosCache.y < 0) vMousePosCache.y = 0;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateMouseState(int32_t button, bool state)
	{
		pMouseNewState[button] = state;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateKeyState(int32_t key, bool state)
	{
		pKeyNewState[key] = state;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateMouseFocus(bool state)
	{
		bHasMouseFocus = state;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_UpdateKeyFocus(bool state)
	{
		bHasInputFocus = state;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_Terminate()
	{
		bAtomActive = false;
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::EngineThread()
	{
		// Allow platform to do stuff here if needed, since its now in the
		// context of this thread
		if (platform->ThreadStartUp() == olc::FAIL)	return;

		// Do engine context specific initialisation
		olc_PrepareEngine();

		// Create user resources as part of this thread
		if (!OnUserCreate()) bAtomActive = false;

		while (bAtomActive)
		{
			// Run as fast as possible
			while (bAtomActive) { olc_CoreUpdate(); }

			// Allow the user to free resources if they have overrided the destroy function
			if (!OnUserDestroy())
			{
				// User denied destroy for some reason, so continue running
				bAtomActive = true;
			}
		}

		platform->ThreadCleanUp();
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_PrepareEngine()
	{
		// Start OpenGL, the context is owned by the game thread
		if (platform->CreateGraphics(bFullScreen, bEnableVSYNC, vViewPos, vViewSize) == olc::FAIL) return;

		// Construct default font sheet
		olc_ConstructFontSheet();

		// Create Primary Layer "0"
		CreateLayer();
		vLayers[0].bUpdate = true;
		vLayers[0].bShow = true;
		SetDrawTarget(nullptr);

		m_tp1 = std::chrono::system_clock::now();
		m_tp2 = std::chrono::system_clock::now();
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_CoreUpdate()
	{
		// Handle Timing
		m_tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = m_tp2 - m_tp1;
		m_tp1 = m_tp2;

		// Our time per frame coefficient
		float fElapsedTime = elapsedTime.count();

		// Some platforms will need to check for events
		platform->HandleSystemEvent();

		// Compare hardware input states from previous frame
		auto ScanHardware = [&](HWButton* pKeys, bool* pStateOld, bool* pStateNew, uint32_t nKeyCount)
		{
			for (uint32_t i = 0; i < nKeyCount; i++)
			{
				pKeys[i].bPressed = false;
				pKeys[i].bReleased = false;
				if (pStateNew[i] != pStateOld[i])
				{
					if (pStateNew[i])
					{
						pKeys[i].bPressed = !pKeys[i].bHeld;
						pKeys[i].bHeld = true;
					}
					else
					{
						pKeys[i].bReleased = true;
						pKeys[i].bHeld = false;
					}
				}
				pStateOld[i] = pStateNew[i];
			}
		};

		ScanHardware(pKeyboardState, pKeyOldState, pKeyNewState, 256);
		ScanHardware(pMouseState, pMouseOldState, pMouseNewState, nMouseButtons);

		// Cache mouse coordinates so they remain consistent during frame
		vMousePos = vMousePosCache;
		nMouseWheelDelta = nMouseWheelDeltaCache;
		nMouseWheelDeltaCache = 0;

		renderer->ClearBuffer(olc::BLACK, true);

		// Handle Frame Update
		if (!OnUserUpdate(fElapsedTime))
			bAtomActive = false;

		// Display Frame
		renderer->UpdateViewport(vViewPos, vViewSize);
		renderer->ClearBuffer(olc::BLACK, true);

		// Layer 0 must always exist
		vLayers[0].bUpdate = true;
		vLayers[0].bShow = true;
		renderer->PrepareDrawing();

		for (auto layer = vLayers.rbegin(); layer != vLayers.rend(); ++layer)
		{
			if (layer->bShow)
			{
				if (layer->funcHook == nullptr)
				{
					renderer->ApplyTexture(layer->nResID);
					if (layer->bUpdate)
					{
						renderer->UpdateTexture(layer->nResID, layer->pDrawTarget);
						layer->bUpdate = false;
					}

					renderer->DrawLayerQuad(layer->vOffset, layer->vScale, layer->tint);

					// Display Decals in order for this layer
					for (auto& decal : layer->vecDecalInstance)
						renderer->DrawDecalQuad(decal);
					layer->vecDecalInstance.clear();
				}
				else
				{
					// Mwa ha ha.... Have Fun!!!
					layer->funcHook();
				}
			}
		}

		// Present Graphics to screen
		renderer->DisplayFrame();

		// Update Title Bar
		fFrameTimer += fElapsedTime;
		nFrameCount++;
		if (fFrameTimer >= 1.0f)
		{
			nLastFPS = nFrameCount;
			fFrameTimer -= 1.0f;
			std::string sTitle = "Pixel Game Engine Fork - " + sAppName + " - FPS: " + std::to_string(nFrameCount);
			platform->SetWindowTitle(sTitle);
			nFrameCount = 0;
		}
	}
	//-------------------------------------------------------------------------

	void PixelGameEngine::olc_ConstructFontSheet()
	{
		std::string data;
		data += "?Q`0001oOch0o01o@F40o0<AGD4090LAGD<090@A7ch0?00O7Q`0600>00000000";
		data += "O000000nOT0063Qo4d8>?7a14Gno94AA4gno94AaOT0>o3`oO400o7QN00000400";
		data += "Of80001oOg<7O7moBGT7O7lABET024@aBEd714AiOdl717a_=TH013Q>00000000";
		data += "720D000V?V5oB3Q_HdUoE7a9@DdDE4A9@DmoE4A;Hg]oM4Aj8S4D84@`00000000";
		data += "OaPT1000Oa`^13P1@AI[?g`1@A=[OdAoHgljA4Ao?WlBA7l1710007l100000000";
		data += "ObM6000oOfMV?3QoBDD`O7a0BDDH@5A0BDD<@5A0BGeVO5ao@CQR?5Po00000000";
		data += "Oc``000?Ogij70PO2D]??0Ph2DUM@7i`2DTg@7lh2GUj?0TO0C1870T?00000000";
		data += "70<4001o?P<7?1QoHg43O;`h@GT0@:@LB@d0>:@hN@L0@?aoN@<0O7ao0000?000";
		data += "OcH0001SOglLA7mg24TnK7ln24US>0PL24U140PnOgl0>7QgOcH0K71S0000A000";
		data += "00H00000@Dm1S007@DUSg00?OdTnH7YhOfTL<7Yh@Cl0700?@Ah0300700000000";
		data += "<008001QL00ZA41a@6HnI<1i@FHLM81M@@0LG81?O`0nC?Y7?`0ZA7Y300080000";
		data += "O`082000Oh0827mo6>Hn?Wmo?6HnMb11MP08@C11H`08@FP0@@0004@000000000";
		data += "00P00001Oab00003OcKP0006@6=PMgl<@440MglH@000000`@000001P00000000";
		data += "Ob@8@@00Ob@8@Ga13R@8Mga172@8?PAo3R@827QoOb@820@0O`0007`0000007P0";
		data += "O`000P08Od400g`<3V=P0G`673IP0`@3>1`00P@6O`P00g`<O`000GP800000000";
		data += "?P9PL020O`<`N3R0@E4HC7b0@ET<ATB0@@l6C4B0O`H3N7b0?P01L3R000000020";

		fontSprite = new olc::Sprite(128, 48);
		int px = 0, py = 0;
		for (int b = 0; b < 1024; b += 4)
		{
			uint32_t sym1 = (uint32_t)data[b + 0] - 48;
			uint32_t sym2 = (uint32_t)data[b + 1] - 48;
			uint32_t sym3 = (uint32_t)data[b + 2] - 48;
			uint32_t sym4 = (uint32_t)data[b + 3] - 48;
			uint32_t r = sym1 << 18 | sym2 << 12 | sym3 << 6 | sym4;

			for (int i = 0; i < 24; i++)
			{
				int k = r & (1 << i) ? 255 : 0;
				fontSprite->SetPixel(px, py, olc::Pixel(k, k, k, k));
				if (++py == 48) { px++; py = 0; }
			}
		}

		fontDecal = new olc::Decal(fontSprite);
	}
	//-------------------------------------------------------------------------

	// Need a couple of statics as these are singleton instances
	// read from multiple locations
	std::atomic<bool> PixelGameEngine::bAtomActive{ false };
	olc::PixelGameEngine* olc::PGEX::pge        = nullptr;
	olc::PixelGameEngine* olc::Platform::ptrPGE = nullptr;
	olc::PixelGameEngine* olc::Renderer::ptrPGE = nullptr;
	//-------------------------------------------------------------------------
};
//-------------------------------------------------------------------------

namespace olc
{
	void PixelGameEngine::olc_ConfigureSystem()
	{
		platform = Platform::createPlatform();
		Renderer::create();

		//// Associate components with PGE instance
		platform->ptrPGE = this;
		renderer->ptrPGE = this;
	}
}
//-------------------------------------------------------------------------

// O----------------------------------------------------------------------O
// | END OF OLC_PGE_APPLICATION                                           |
// O----------------------------------------------------------------------O
