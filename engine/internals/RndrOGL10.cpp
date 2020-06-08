// O--------------------------------------------------------------------------O
// | START RENDERER: OpenGL 1.0 (the original, the best...)                   |
// O--------------------------------------------------------------------------O

#include "../pgeDefs.h"
//-----------------------------------------------------------------------------
#if defined(OLC_GFX_OPENGL10)
#if defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
typedef BOOL(WINAPI wglSwapInterval_t) (int interval);
static wglSwapInterval_t* wglSwapInterval = nullptr;
typedef HDC glDeviceContext_t;
typedef HGLRC glRenderContext_t;
#endif

#if defined(__linux__) || defined(__FreeBSD__)
#include <GL/gl.h>
namespace X11
{
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
}
#include <png.h>
typedef int(glSwapInterval_t)(X11::Display* dpy, X11::GLXDrawable drawable, int interval);
static glSwapInterval_t* glSwapIntervalEXT;
typedef X11::GLXContext glDeviceContext_t;
typedef X11::GLXContext glRenderContext_t;
#endif
//-----------------------------------------------------------------------------
#include "../pgeCore.h"
//-----------------------------------------------------------------------------

namespace olc
{
	//-------------------------------------------------------------------------

	class Renderer_OGL10 : public olc::Renderer
	{
	private:
		glDeviceContext_t glDeviceContext = 0;
		glRenderContext_t glRenderContext = 0;

#if defined(__linux__) || defined(__FreeBSD__)
		X11::Display* olc_Display = nullptr;
		X11::Window* olc_Window = nullptr;
		X11::XVisualInfo* olc_VisualInfo = nullptr;
#endif

	public:
		//---------------------------------------------------------------------
		void PrepareDevice() override {}
		//---------------------------------------------------------------------

		olc::rcode CreateDevice(std::vector<void*> params, bool bFullScreen, bool bVSYNC) override
		{
#if defined(_WIN32)
			// Create Device Context
			glDeviceContext = GetDC((HWND)(params[0]));
			PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR), 1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				PFD_MAIN_PLANE, 0, 0, 0, 0
			};

			int pf = 0;
			if (!(pf = ChoosePixelFormat(glDeviceContext, &pfd))) return olc::FAIL;
			SetPixelFormat(glDeviceContext, pf, &pfd);

			if (!(glRenderContext = wglCreateContext(glDeviceContext))) return olc::FAIL;
			wglMakeCurrent(glDeviceContext, glRenderContext);

			// Remove Frame cap
			wglSwapInterval = (wglSwapInterval_t*)wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapInterval && !bVSYNC) wglSwapInterval(0);
#endif

#if defined(__linux__) || defined(__FreeBSD__)
			using namespace X11;
			// Linux has tighter coupling between OpenGL and X11, so we store
			// various "platform" handles in the renderer
			olc_Display = (X11::Display*)(params[0]);
			olc_Window = (X11::Window*)(params[1]);
			olc_VisualInfo = (X11::XVisualInfo*)(params[2]);

			glDeviceContext = glXCreateContext(olc_Display, olc_VisualInfo, nullptr, GL_TRUE);
			glXMakeCurrent(olc_Display, *olc_Window, glDeviceContext);

			XWindowAttributes gwa;
			XGetWindowAttributes(olc_Display, *olc_Window, &gwa);
			glViewport(0, 0, gwa.width, gwa.height);

			glSwapIntervalEXT = nullptr;
			glSwapIntervalEXT = (glSwapInterval_t*)glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT");

			if (glSwapIntervalEXT == nullptr && !bVSYNC)
			{
				printf("NOTE: Could not disable VSYNC, glXSwapIntervalEXT() was not found!\n");
				printf("      Don't worry though, things will still work, it's just the\n");
				printf("      frame rate will be capped to your monitors refresh rate - javidx9\n");
			}

			if (glSwapIntervalEXT != nullptr && !bVSYNC)
				glSwapIntervalEXT(olc_Display, *olc_Window, 0);
#endif		

			glEnable(GL_TEXTURE_2D); // Turn on texturing
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
			return olc::rcode::OK;
		}
		//---------------------------------------------------------------------

		olc::rcode DestroyDevice() override
		{
#if defined(_WIN32)
			wglDeleteContext(glRenderContext);
#endif

#if defined(__linux__) || defined(__FreeBSD__)
			glXMakeCurrent(olc_Display, None, NULL);
			glXDestroyContext(olc_Display, glDeviceContext);
#endif
			return olc::rcode::OK;
		}
		//---------------------------------------------------------------------

		void DisplayFrame() override
		{
#if defined(_WIN32)
			SwapBuffers(glDeviceContext);
#endif	

#if defined(__linux__) || defined(__FreeBSD__)
			X11::glXSwapBuffers(olc_Display, *olc_Window);
#endif		
		}
		//---------------------------------------------------------------------

		void PrepareDrawing() override
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		//---------------------------------------------------------------------

		void DrawLayerQuad(const olc::vf2d& offset, const olc::vf2d& scale, const olc::Pixel tint) override
		{
			glBegin(GL_QUADS);
			glColor4ub(tint.r, tint.g, tint.b, tint.a);
			glTexCoord2f(0.0f * scale.x + offset.x, 1.0f * scale.y + offset.y);
			glVertex3f(-1.0f /*+ vSubPixelOffset.x*/, -1.0f /*+ vSubPixelOffset.y*/, 0.0f);
			glTexCoord2f(0.0f * scale.x + offset.x, 0.0f * scale.y + offset.y);
			glVertex3f(-1.0f /*+ vSubPixelOffset.x*/, 1.0f /*+ vSubPixelOffset.y*/, 0.0f);
			glTexCoord2f(1.0f * scale.x + offset.x, 0.0f * scale.y + offset.y);
			glVertex3f(1.0f /*+ vSubPixelOffset.x*/, 1.0f /*+ vSubPixelOffset.y*/, 0.0f);
			glTexCoord2f(1.0f * scale.x + offset.x, 1.0f * scale.y + offset.y);
			glVertex3f(1.0f /*+ vSubPixelOffset.x*/, -1.0f /*+ vSubPixelOffset.y*/, 0.0f);
			glEnd();
		}
		//---------------------------------------------------------------------

		void DrawDecalQuad(const olc::DecalInstance& decal) override
		{
			glBindTexture(GL_TEXTURE_2D, decal.decal->id);
			glBegin(GL_QUADS);
			glColor4ub(decal.tint.r, decal.tint.g, decal.tint.b, decal.tint.a);
			glTexCoord4f(decal.uv[0].x, decal.uv[0].y, 0.0f, decal.w[0]); glVertex2f(decal.pos[0].x, decal.pos[0].y);
			glTexCoord4f(decal.uv[1].x, decal.uv[1].y, 0.0f, decal.w[1]); glVertex2f(decal.pos[1].x, decal.pos[1].y);
			glTexCoord4f(decal.uv[2].x, decal.uv[2].y, 0.0f, decal.w[2]); glVertex2f(decal.pos[2].x, decal.pos[2].y);
			glTexCoord4f(decal.uv[3].x, decal.uv[3].y, 0.0f, decal.w[3]); glVertex2f(decal.pos[3].x, decal.pos[3].y);
			glEnd();
		}
		//---------------------------------------------------------------------

		uint32_t CreateTexture(const uint32_t width, const uint32_t height) override
		{
			uint32_t id = 0;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			return id;
		}
		//---------------------------------------------------------------------

		uint32_t DeleteTexture(const uint32_t id) override
		{
			glDeleteTextures(1, &id);
			return id;
		}
		//---------------------------------------------------------------------

		void UpdateTexture(uint32_t id, olc::ISprite* spr) override
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spr->width, spr->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, spr->GetData());
		}
		//---------------------------------------------------------------------

		void ApplyTexture(uint32_t id) override
		{
			glBindTexture(GL_TEXTURE_2D, id);
		}
		//---------------------------------------------------------------------

		void ClearBuffer(olc::Pixel p, bool bDepth) override
		{
			glClearColor(float(p.r) / 255.0f, float(p.g) / 255.0f, float(p.b) / 255.0f, float(p.a) / 255.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			if (bDepth) glClear(GL_DEPTH_BUFFER_BIT);
		}
		//---------------------------------------------------------------------

		void UpdateViewport(const olc::vi2d& pos, const olc::vi2d& size) override
		{
			glViewport(pos.x, pos.y, size.x, size.y);
		}
		//---------------------------------------------------------------------
	};
	//-------------------------------------------------------------------------

	Renderer* Renderer::create()
	{
		if(!m_rend)
			m_rend = std::make_unique<Renderer_OGL10>();
		return m_rend.get();
	}
	//-------------------------------------------------------------------------
	std::unique_ptr<Renderer> Renderer::m_rend;
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif
// O--------------------------------------------------------------------------O
// | END RENDERER: OpenGL 1.0 (the original, the best...)                     |
// O--------------------------------------------------------------------------O
