//-----------------------------------------------------------------------------
// O--------------------------------------------------------------------------O
// | START PLATFORM: LINUX                                                    |
// O--------------------------------------------------------------------------O
#if defined(__linux__) || defined(__FreeBSD__)
//-----------------------------------------------------------------------------
#include "../pgeInternals.h"
//-----------------------------------------------------------------------------

namespace olc
{
	//-------------------------------------------------------------------------

	class Platform_Linux : public olc::Platform
	{
	private:
		X11::Display* olc_Display = nullptr;
		X11::Window					 olc_WindowRoot;
		X11::Window					 olc_Window;
		X11::XVisualInfo* olc_VisualInfo;
		X11::Colormap                olc_ColourMap;
		X11::XSetWindowAttributes    olc_SetWindowAttribs;

	public:
		virtual olc::rcode ApplicationStartUp() override
		{
			return olc::rcode::OK;
		}

		virtual olc::rcode ApplicationCleanUp() override
		{
			return olc::rcode::OK;
		}

		virtual olc::rcode ThreadStartUp() override
		{
			return olc::rcode::OK;
		}

		virtual olc::rcode ThreadCleanUp() override
		{
			renderer->DestroyDevice();
			return olc::OK;
		}

		virtual olc::rcode CreateGraphics(bool bFullScreen, bool bEnableVSYNC, const olc::vi2d& vViewPos, const olc::vi2d& vViewSize) override
		{
			if (renderer->CreateDevice({ olc_Display, &olc_Window, olc_VisualInfo }, bFullScreen, bEnableVSYNC) == olc::rcode::OK)
			{
				renderer->UpdateViewport(vViewPos, vViewSize);
				return olc::rcode::OK;
			}
			else
				return olc::rcode::FAIL;
		}

		virtual olc::rcode CreateWindowPane(const olc::vi2d& vWindowPos, olc::vi2d& vWindowSize, bool bFullScreen) override
		{
			using namespace X11;
			XInitThreads();

			// Grab the deafult display and window
			olc_Display = XOpenDisplay(NULL);
			olc_WindowRoot = DefaultRootWindow(olc_Display);

			// Based on the display capabilities, configure the appearance of the window
			GLint olc_GLAttribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
			olc_VisualInfo = glXChooseVisual(olc_Display, 0, olc_GLAttribs);
			olc_ColourMap = XCreateColormap(olc_Display, olc_WindowRoot, olc_VisualInfo->visual, AllocNone);
			olc_SetWindowAttribs.colormap = olc_ColourMap;

			// Register which events we are interested in receiving
			olc_SetWindowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask;

			// Create the window
			olc_Window = XCreateWindow(olc_Display, olc_WindowRoot, vWindowPos.x, vWindowPos.y,
				vWindowSize.x, vWindowSize.y,
				0, olc_VisualInfo->depth, InputOutput, olc_VisualInfo->visual,
				CWColormap | CWEventMask, &olc_SetWindowAttribs);

			Atom wmDelete = XInternAtom(olc_Display, "WM_DELETE_WINDOW", true);
			XSetWMProtocols(olc_Display, olc_Window, &wmDelete, 1);

			XMapWindow(olc_Display, olc_Window);
			XStoreName(olc_Display, olc_Window, "OneLoneCoder.com - Pixel Game Engine");

			if (bFullScreen) // Thanks DragonEye, again :D
			{
				Atom wm_state;
				Atom fullscreen;
				wm_state = XInternAtom(olc_Display, "_NET_WM_STATE", False);
				fullscreen = XInternAtom(olc_Display, "_NET_WM_STATE_FULLSCREEN", False);
				XEvent xev{ 0 };
				xev.type = ClientMessage;
				xev.xclient.window = olc_Window;
				xev.xclient.message_type = wm_state;
				xev.xclient.format = 32;
				xev.xclient.data.l[0] = (bFullScreen ? 1 : 0);   // the action (0: off, 1: on, 2: toggle)
				xev.xclient.data.l[1] = fullscreen;             // first property to alter
				xev.xclient.data.l[2] = 0;                      // second property to alter
				xev.xclient.data.l[3] = 0;                      // source indication
				XMapWindow(olc_Display, olc_Window);
				XSendEvent(olc_Display, DefaultRootWindow(olc_Display), False,
					SubstructureRedirectMask | SubstructureNotifyMask, &xev);
				XFlush(olc_Display);
				XWindowAttributes gwa;
				XGetWindowAttributes(olc_Display, olc_Window, &gwa);
				vWindowSize.x = gwa.width;
				vWindowSize.y = gwa.height;
			}

			// Create Keyboard Mapping
			mapKeys[0x00] = Key::NONE;
			mapKeys[0x61] = Key::A; mapKeys[0x62] = Key::B; mapKeys[0x63] = Key::C; mapKeys[0x64] = Key::D; mapKeys[0x65] = Key::E;
			mapKeys[0x66] = Key::F; mapKeys[0x67] = Key::G; mapKeys[0x68] = Key::H; mapKeys[0x69] = Key::I; mapKeys[0x6A] = Key::J;
			mapKeys[0x6B] = Key::K; mapKeys[0x6C] = Key::L; mapKeys[0x6D] = Key::M; mapKeys[0x6E] = Key::N; mapKeys[0x6F] = Key::O;
			mapKeys[0x70] = Key::P; mapKeys[0x71] = Key::Q; mapKeys[0x72] = Key::R; mapKeys[0x73] = Key::S; mapKeys[0x74] = Key::T;
			mapKeys[0x75] = Key::U; mapKeys[0x76] = Key::V; mapKeys[0x77] = Key::W; mapKeys[0x78] = Key::X; mapKeys[0x79] = Key::Y;
			mapKeys[0x7A] = Key::Z;

			mapKeys[XK_F1] = Key::F1; mapKeys[XK_F2] = Key::F2; mapKeys[XK_F3] = Key::F3; mapKeys[XK_F4] = Key::F4;
			mapKeys[XK_F5] = Key::F5; mapKeys[XK_F6] = Key::F6; mapKeys[XK_F7] = Key::F7; mapKeys[XK_F8] = Key::F8;
			mapKeys[XK_F9] = Key::F9; mapKeys[XK_F10] = Key::F10; mapKeys[XK_F11] = Key::F11; mapKeys[XK_F12] = Key::F12;

			mapKeys[XK_Down] = Key::DOWN; mapKeys[XK_Left] = Key::LEFT; mapKeys[XK_Right] = Key::RIGHT; mapKeys[XK_Up] = Key::UP;
			mapKeys[XK_KP_Enter] = Key::ENTER; mapKeys[XK_Return] = Key::ENTER;

			mapKeys[XK_BackSpace] = Key::BACK; mapKeys[XK_Escape] = Key::ESCAPE; mapKeys[XK_Linefeed] = Key::ENTER;	mapKeys[XK_Pause] = Key::PAUSE;
			mapKeys[XK_Scroll_Lock] = Key::SCROLL; mapKeys[XK_Tab] = Key::TAB; mapKeys[XK_Delete] = Key::DEL; mapKeys[XK_Home] = Key::HOME;
			mapKeys[XK_End] = Key::END; mapKeys[XK_Page_Up] = Key::PGUP; mapKeys[XK_Page_Down] = Key::PGDN;	mapKeys[XK_Insert] = Key::INS;
			mapKeys[XK_Shift_L] = Key::SHIFT; mapKeys[XK_Shift_R] = Key::SHIFT; mapKeys[XK_Control_L] = Key::CTRL; mapKeys[XK_Control_R] = Key::CTRL;
			mapKeys[XK_space] = Key::SPACE; mapKeys[XK_period] = Key::PERIOD;

			mapKeys[XK_0] = Key::K0; mapKeys[XK_1] = Key::K1; mapKeys[XK_2] = Key::K2; mapKeys[XK_3] = Key::K3; mapKeys[XK_4] = Key::K4;
			mapKeys[XK_5] = Key::K5; mapKeys[XK_6] = Key::K6; mapKeys[XK_7] = Key::K7; mapKeys[XK_8] = Key::K8; mapKeys[XK_9] = Key::K9;

			mapKeys[XK_KP_0] = Key::NP0; mapKeys[XK_KP_1] = Key::NP1; mapKeys[XK_KP_2] = Key::NP2; mapKeys[XK_KP_3] = Key::NP3; mapKeys[XK_KP_4] = Key::NP4;
			mapKeys[XK_KP_5] = Key::NP5; mapKeys[XK_KP_6] = Key::NP6; mapKeys[XK_KP_7] = Key::NP7; mapKeys[XK_KP_8] = Key::NP8; mapKeys[XK_KP_9] = Key::NP9;
			mapKeys[XK_KP_Multiply] = Key::NP_MUL; mapKeys[XK_KP_Add] = Key::NP_ADD; mapKeys[XK_KP_Divide] = Key::NP_DIV; mapKeys[XK_KP_Subtract] = Key::NP_SUB; mapKeys[XK_KP_Decimal] = Key::NP_DECIMAL;

			return olc::OK;
		}

		virtual olc::rcode SetWindowTitle(const std::string& s) override
		{
			X11::XStoreName(olc_Display, olc_Window, s.c_str());
			return olc::OK;
		}

		virtual olc::rcode StartSystemEventLoop() override
		{
			return olc::OK;
		}

		virtual olc::rcode HandleSystemEvent() override
		{
			using namespace X11;
			// Handle Xlib Message Loop - we do this in the
			// same thread that OpenGL was created so we dont
			// need to worry too much about multithreading with X11
			XEvent xev;
			while (XPending(olc_Display))
			{
				XNextEvent(olc_Display, &xev);
				if (xev.type == Expose)
				{
					XWindowAttributes gwa;
					XGetWindowAttributes(olc_Display, olc_Window, &gwa);
					ptrPGE->olc_UpdateWindowSize(gwa.width, gwa.height);
				}
				else if (xev.type == ConfigureNotify)
				{
					XConfigureEvent xce = xev.xconfigure;
					ptrPGE->olc_UpdateWindowSize(xce.width, xce.height);
				}
				else if (xev.type == KeyPress)
				{
					KeySym sym = XLookupKeysym(&xev.xkey, 0);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], true);
					XKeyEvent* e = (XKeyEvent*)&xev; // Because DragonEye loves numpads
					XLookupString(e, NULL, 0, &sym, NULL);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], true);
				}
				else if (xev.type == KeyRelease)
				{
					KeySym sym = XLookupKeysym(&xev.xkey, 0);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], false);
					XKeyEvent* e = (XKeyEvent*)&xev;
					XLookupString(e, NULL, 0, &sym, NULL);
					ptrPGE->olc_UpdateKeyState(mapKeys[sym], false);
				}
				else if (xev.type == ButtonPress)
				{
					switch (xev.xbutton.button)
					{
					case 1:	ptrPGE->olc_UpdateMouseState(0, true); break;
					case 2:	ptrPGE->olc_UpdateMouseState(2, true); break;
					case 3:	ptrPGE->olc_UpdateMouseState(1, true); break;
					case 4:	ptrPGE->olc_UpdateMouseWheel(120); break;
					case 5:	ptrPGE->olc_UpdateMouseWheel(-120); break;
					default: break;
					}
				}
				else if (xev.type == ButtonRelease)
				{
					switch (xev.xbutton.button)
					{
					case 1:	ptrPGE->olc_UpdateMouseState(0, false); break;
					case 2:	ptrPGE->olc_UpdateMouseState(2, false); break;
					case 3:	ptrPGE->olc_UpdateMouseState(1, false); break;
					default: break;
					}
				}
				else if (xev.type == MotionNotify)
				{
					ptrPGE->olc_UpdateMouse(xev.xmotion.x, xev.xmotion.y);
				}
				else if (xev.type == FocusIn)
				{
					ptrPGE->olc_UpdateKeyFocus(true);
				}
				else if (xev.type == FocusOut)
				{
					ptrPGE->olc_UpdateKeyFocus(false);
				}
				else if (xev.type == ClientMessage)
				{
					ptrPGE->olc_Terminate();
				}
			}
			return olc::OK;
		}
	};
	//-------------------------------------------------------------------------

	void pngReadStream(png_structp pngPtr, png_bytep data, png_size_t length)
	{
		png_voidp a = png_get_io_ptr(pngPtr);
		((std::istream*)a)->read((char*)data, length);
	}
	//-------------------------------------------------------------------------

	olc::rcode Sprite::LoadFromFile(const std::string& sImageFile, olc::ResourcePack* pack)
	{
		UNUSED(pack);
		////////////////////////////////////////////////////////////////////////////
		// Use libpng, Thanks to Guillaume Cottenceau
		// https://gist.github.com/niw/5963798

		// Also reading png from streams
		// http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/

		png_structp png;
		png_infop info;

		auto loadPNG = [&]()
		{
			png_read_info(png, info);
			png_byte color_type;
			png_byte bit_depth;
			png_bytep* row_pointers;
			width = png_get_image_width(png, info);
			height = png_get_image_height(png, info);
			color_type = png_get_color_type(png, info);
			bit_depth = png_get_bit_depth(png, info);
			if (bit_depth == 16) png_set_strip_16(png);
			if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
			if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)	png_set_expand_gray_1_2_4_to_8(png);
			if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
			if (color_type == PNG_COLOR_TYPE_RGB ||
				color_type == PNG_COLOR_TYPE_GRAY ||
				color_type == PNG_COLOR_TYPE_PALETTE)
				png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
			if (color_type == PNG_COLOR_TYPE_GRAY ||
				color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
				png_set_gray_to_rgb(png);
			png_read_update_info(png, info);
			row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
			for (int y = 0; y < height; y++) {
				row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
			}
			png_read_image(png, row_pointers);
			////////////////////////////////////////////////////////////////////////////
			// Create sprite array
			pColData = new Pixel[width * height];
			// Iterate through image rows, converting into sprite format
			for (int y = 0; y < height; y++)
			{
				png_bytep row = row_pointers[y];
				for (int x = 0; x < width; x++)
				{
					png_bytep px = &(row[x * 4]);
					SetPixel(x, y, Pixel(px[0], px[1], px[2], px[3]));
				}
			}

			for (int y = 0; y < height; y++) // Thanks maksym33
				free(row_pointers[y]);
			free(row_pointers);
			png_destroy_read_struct(&png, &info, nullptr);
		};

		png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png) goto fail_load;

		info = png_create_info_struct(png);
		if (!info) goto fail_load;

		if (setjmp(png_jmpbuf(png))) goto fail_load;

		if (pack == nullptr)
		{
			FILE* f = fopen(sImageFile.c_str(), "rb");
			if (!f) return olc::NO_FILE;
			png_init_io(png, f);
			loadPNG();
			fclose(f);
		}
		else
		{
			ResourceBuffer rb = pack->GetFileBuffer(sImageFile);
			std::istream is(&rb);
			png_set_read_fn(png, (png_voidp)&is, pngReadStream);
			loadPNG();
		}

		return olc::OK;

	fail_load:
		width = 0;
		height = 0;
		pColData = nullptr;
		return olc::FAIL;
	}
	//-------------------------------------------------------------------------

	std::unique_ptr<Platform> olc::Platform::createPlatform()
	{
		auto rtn = std::make_unique<Platform_Linux>();
		return rtn;
	}
}
#endif
// O--------------------------------------------------------------------------O
// | END PLATFORM: LINUX                                                      |
// O--------------------------------------------------------------------------O
//-----------------------------------------------------------------------------
