/*
	olcPGEX_Sound.h

	+-------------------------------------------------------------+
	|         OneLoneCoder Pixel Game Engine Extension            |
	|                       Sound - v0.3                          |
	+-------------------------------------------------------------+

	What is this?
	~~~~~~~~~~~~~
	This is an extension to the olcPixelGameEngine, which provides
	sound generation and wave playing routines.

	Special Thanks:
	~~~~~~~~~~~~~~~	
	Slavka - For entire non-windows system back end!
	Gorbit99 - Testing, Bug Fixes
	Cyberdroid - Testing, Bug Fixes
	Dragoneye - Testing
	Puol - Testing

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018 - 2019 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Homepage:	https://www.onelonecoder.com
	Patreon:	https://www.patreon.com/javidx9

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2019
*/

#pragma once

#include "olcPixelGameEngine.h"
#include <istream>
#include <cstring>
#include <climits>
#include <algorithm>

#undef min
#undef max

// Choose a default sound backend
#if !defined(USE_ALSA) && !defined(USE_OPENAL) && !defined(USE_WINDOWS)
#ifdef __linux__
#define USE_ALSA
#endif

#ifdef __EMSCRIPTEN__
#define USE_OPENAL
#endif

#ifdef _WIN32
#define USE_WINDOWS
#endif

#endif

#ifdef USE_ALSA
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

#ifdef USE_WINDOWS
#include <condition_variable>
#include <Windows.h>
#endif

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <queue>
#endif

#pragma pack(push, 1)
typedef struct {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	uint16_t cbSize;
} OLC_WAVEFORMATEX;
#pragma pack(pop)

namespace olc
{
	// Container class for Sound
	class SOUND : public PGEX
	{
	public:
		class AudioSample
		{
		public:
			AudioSample();
			AudioSample(std::string sWavFile, olc::ResourcePack *pack = nullptr);
			olc::rcode LoadFromFile(std::string sWavFile, olc::ResourcePack *pack = nullptr);

		public:
			OLC_WAVEFORMATEX wavHeader;
			float *fSample = nullptr;
			long nSamples = 0;
			int nChannels = 0;
			bool bSampleValid = false;
		};

		struct sCurrentlyPlayingSample
		{
			int nAudioSampleID = 0;
			long nSamplePosition = 0;
			bool bFinished = false;
			bool bLoop = false;
			bool bFlagForStop = false;
		};

		static std::list<sCurrentlyPlayingSample> listActiveSamples;

	public:
		static bool InitialiseAudio(unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512);
		static bool DestroyAudio();
		static void SetUserSynthFunction(std::function<float(int, float, float)> func);
		static void SetUserFilterFunction(std::function<float(int, float, float)> func);

	public:
		static int LoadAudioSample(std::string sWavFile, ResourcePack *pack = nullptr);
		static void PlaySample(int id, bool bLoop = false);
		static void StopSample(int id);
		static void StopAll();
		static float GetMixerOutput(int nChannel, float fGlobalTime, float fTimeStep);


	private:
#ifdef USE_WINDOWS // Windows specific sound management
		static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockCount;
		static unsigned int m_nBlockSamples;
		static unsigned int m_nBlockCurrent;
		static short* m_pBlockMemory;
		static WAVEHDR *m_pWaveHeaders;
		static HWAVEOUT m_hwDevice;
		static std::atomic<unsigned int> m_nBlockFree;
		static std::condition_variable m_cvBlockNotZero;
		static std::mutex m_muxBlockNotZero;
#endif

#ifdef USE_ALSA
		static snd_pcm_t *m_pPCM;
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockSamples;
		static short* m_pBlockMemory;
#endif

#ifdef USE_OPENAL
		static std::queue<ALuint> m_qAvailableBuffers;
		static ALuint *m_pBuffers;
		static ALuint m_nSource;
		static ALCdevice *m_pDevice;
		static ALCcontext *m_pContext;
		static unsigned int m_nSampleRate;
		static unsigned int m_nChannels;
		static unsigned int m_nBlockCount;
		static unsigned int m_nBlockSamples;
		static short* m_pBlockMemory;
#endif

		static void AudioThread();
		static std::thread m_AudioThread;
		static std::atomic<bool> m_bAudioThreadActive;
		static std::atomic<float> m_fGlobalTime;
		static std::function<float(int, float, float)> funcUserSynth;
		static std::function<float(int, float, float)> funcUserFilter;
	};
}
