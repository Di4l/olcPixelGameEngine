//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
#include "pgeTypes.h"
#include "pgeSprite.h"
#include "pgeDecal.h"
//-----------------------------------------------------------------------------

namespace olc
{
	//-------------------------------------------------------------------------

	struct LayerDesc
	{
		olc::vf2d                  vOffset     = { 0, 0 };
		olc::vf2d                  vScale      = { 1, 1 };
		bool                       bShow       = false;
		bool                       bUpdate     = false;
		olc::ISprite*              pDrawTarget = nullptr;
		uint32_t                   nResID      = 0;
		std::vector<DecalInstance> vecDecalInstance;
		olc::Pixel                 tint        = olc::WHITE;
		std::function<void()>      funcHook    = nullptr;
	};
	//-------------------------------------------------------------------------

	class Renderer
	{
	public:
		virtual void       PrepareDevice() = 0;
		virtual olc::rcode CreateDevice(std::vector<void*> params, bool bFullScreen, bool bVSYNC) = 0;
		virtual olc::rcode DestroyDevice() = 0;
		virtual void       DisplayFrame() = 0;
		virtual void       PrepareDrawing() = 0;
		virtual void       DrawLayerQuad(const olc::vf2d& offset, const olc::vf2d& scale, const olc::Pixel tint) = 0;
		virtual void       DrawDecalQuad(const olc::DecalInstance& decal) = 0;
		virtual uint32_t   CreateTexture(const uint32_t width, const uint32_t height) = 0;
		virtual void       UpdateTexture(uint32_t id, olc::ISprite* spr) = 0;
		virtual uint32_t   DeleteTexture(const uint32_t id) = 0;
		virtual void       ApplyTexture(uint32_t id) = 0;
		virtual void       UpdateViewport(const olc::vi2d& pos, const olc::vi2d& size) = 0;
		virtual void       ClearBuffer(olc::Pixel p, bool bDepth) = 0;

		static olc::PixelGameEngine* ptrPGE;
		static Renderer* create();
		static Renderer* get() { return m_rend.get(); }
	private:
		static std::unique_ptr<Renderer> m_rend;
	};
	//-------------------------------------------------------------------------

	class Platform
	{
	public:
		virtual olc::rcode ApplicationStartUp() = 0;
		virtual olc::rcode ApplicationCleanUp() = 0;
		virtual olc::rcode ThreadStartUp() = 0;
		virtual olc::rcode ThreadCleanUp() = 0;
		virtual olc::rcode CreateGraphics(bool bFullScreen, bool bEnableVSYNC, const olc::vi2d& vViewPos, const olc::vi2d& vViewSize) = 0;
		virtual olc::rcode CreateWindowPane(const olc::vi2d& vWindowPos, olc::vi2d& vWindowSize, bool bFullScreen) = 0;
		virtual olc::rcode SetWindowTitle(const std::string& s) = 0;
		virtual olc::rcode StartSystemEventLoop() = 0;
		virtual olc::rcode HandleSystemEvent() = 0;

		static olc::PixelGameEngine* ptrPGE;
		static std::unique_ptr<Platform> createPlatform();
	};
	//-------------------------------------------------------------------------
#define renderer	Renderer::get()
	static std::unique_ptr<Platform> platform;
	static std::map<size_t, uint8_t> mapKeys;
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
