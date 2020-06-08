/**
 *    Move several core definitions here
 */

#pragma once

// O------------------------------------------------------------------------------O
// | STANDARD INCLUDES                                                            |
// O------------------------------------------------------------------------------O
#include <cmath>
#include <cstdint>
#include <string>
#include <iostream>
#include <streambuf>
#include <sstream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <fstream>
#include <map>
#include <functional>
#include <algorithm>
#include <array>
#include <cstring>

// O------------------------------------------------------------------------------O
// | COMPILER CONFIGURATION ODDITIES                                              |
// O------------------------------------------------------------------------------O
#define USE_EXPERIMENTAL_FS

#if defined(_WIN32)
#if _MSC_VER >= 1920 && _MSVC_LANG >= 201703L
#undef USE_EXPERIMENTAL_FS
#endif
#endif

#if defined(__linux__) || defined(__MINGW32__) || defined(__EMSCRIPTEN__) || defined(__FreeBSD__)
#if __cplusplus >= 201703L
#undef USE_EXPERIMENTAL_FS
#endif
#endif

#if defined(USE_EXPERIMENTAL_FS)
	// C++14
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace _gfs = std::experimental::filesystem::v1;
#else
	// C++17
#include <filesystem>
namespace _gfs = std::filesystem;
#endif

#if defined(UNICODE) || defined(_UNICODE)
#define olcT(s) L##s
#else
#define olcT(s) s
#endif

#define UNUSED(x) (void)(x)

#if !defined(OLC_GFX_OPENGL33) && !defined(OLC_GFX_DIRECTX10)
#define OLC_GFX_OPENGL10
#endif
