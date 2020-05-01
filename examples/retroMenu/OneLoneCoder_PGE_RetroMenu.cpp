/*
	Easy To Use "Retro Pop-Up Menu System"
	"There's a storm comin'......" - javidx9
	License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018-2020 OneLoneCoder.com
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
	Relevant Video: https://youtu.be/jde1Jq5dF0E
	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Community Blog: https://community.onelonecoder.com
	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2018, 2019, 2020
*/


#include "olcPixelGameEngine.h"
#include "popupMenu/pgexPopupMenu.h" 

constexpr int32_t nPatch = 8;

// Override base class with your custom functionality
class olcRetroPopUpMenu : public olc::PixelGameEngine
{
public:
	olcRetroPopUpMenu()
	{
		sAppName = "Retro Pop-Up Menu";
	}

	olc::Sprite* sprGFX = nullptr;

	olc::popup::Menu    mo;
	olc::popup::Manager mm;

public:
	bool OnUserCreate() override
	{
		sprGFX = new olc::Sprite("./RetroMenu.png");

		mo["main"].SetTable(1, 4);
		mo["main"]["Attack"].SetID(101);

		mo["main"]["Magic"].SetTable(1, 2);

		mo["main"]["Magic"]["White"].SetTable(3, 6);
		auto& menuWhiteMagic = mo["main"]["Magic"]["White"];
		menuWhiteMagic["Cure"].SetID(401);
		menuWhiteMagic["Cura"].SetID(402);
		menuWhiteMagic["Curaga"].SetID(403);
		menuWhiteMagic["Esuna"].SetID(404);

		mo["main"]["Magic"]["Black"].SetTable(3, 4);
		auto& menuBlackMagic = mo["main"]["Magic"]["Black"];
		menuBlackMagic["Fire"].SetID(201);
		menuBlackMagic["Fira"].SetID(202);
		menuBlackMagic["Firaga"].SetID(203);
		menuBlackMagic["Blizzard"].SetID(204);
		menuBlackMagic["Blizzara"].SetID(205).Enable(false);
		menuBlackMagic["Blizzaga"].SetID(206).Enable(false);
		menuBlackMagic["Thunder"].SetID(207);
		menuBlackMagic["Thundara"].SetID(208);
		menuBlackMagic["Thundaga"].SetID(209);
		menuBlackMagic["Quake"].SetID(210);
		menuBlackMagic["Quake2"].SetID(211);
		menuBlackMagic["Quake3"].SetID(212);
		menuBlackMagic["Bio"].SetID(213);
		menuBlackMagic["Bio1"].SetID(214);
		menuBlackMagic["Bio2"].SetID(215);
		menuBlackMagic["Demi"].SetID(216);
		menuBlackMagic["Demi1"].SetID(217);
		menuBlackMagic["Demi2"].SetID(218);

		mo["main"]["Defend"].SetID(102);

		mo["main"]["Items"].SetTable(2, 4).Enable(false);
		mo["main"]["Items"]["Potion"].SetID(301);
		mo["main"]["Items"]["Ether"].SetID(302);
		mo["main"]["Items"]["Elixir"].SetID(303);

		mo["main"]["Escape"].SetID(103);

		mo.Build();

		mm.Open(&mo["main"]);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		olc::popup::Menu* command = nullptr;

		if (GetKey(olc::Key::M).bPressed)    mm.Open(&mo["main"]);

		if (GetKey(olc::Key::UP).bPressed)    mm.OnUp();
		if (GetKey(olc::Key::DOWN).bPressed)  mm.OnDown();
		if (GetKey(olc::Key::LEFT).bPressed)  mm.OnLeft();
		if (GetKey(olc::Key::RIGHT).bPressed) mm.OnRight();
		if (GetKey(olc::Key::SPACE).bPressed) command = mm.OnConfirm();
		if (GetKey(olc::Key::Z).bPressed)     mm.OnBack();

		if (command != nullptr)
		{
			sLastAction = "Selected: " + command->GetName() + " ID: " + std::to_string(command->GetID());
			mm.Close();
		}

		Clear(olc::BLACK);
		mm.Draw(sprGFX, { 30,30 });
		DrawString(10, 200, sLastAction);
		return true;
	}

	std::string sLastAction;
};

int main()
{
	olcRetroPopUpMenu demo;
	if (demo.Construct(384, 240, 4, 4))
		demo.Start();
	return 0;
}