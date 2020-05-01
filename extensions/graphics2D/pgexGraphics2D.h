/*
	olcPGEX_Graphics2D.h

	+-------------------------------------------------------------+
	|         OneLoneCoder Pixel Game Engine Extension            |
	|                Advanced 2D Rendering - v0.5                 |
	+-------------------------------------------------------------+

	What is this?
	~~~~~~~~~~~~~
	This is an extension to the olcPixelGameEngine, which provides
	advanced olc::Sprite manipulation and drawing routines. To use
	it, simply include this header file.

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

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2019
*/

/*
	Matrices stored as [Column][Row] (i.e. x, y)

	|C0R0 C1R0 C2R0|   | x |   | x'|
	|C0R1 C1R1 C2R1| * | y | = | y'|
	|C0R2 C1R2 C2R2|   |1.0|   | - |
*/


#pragma once

#include "olcPixelGameEngine.h"
#include <algorithm>

#undef min
#undef max

namespace olc
{
	// Container class for Advanced 2D Drawing functions
	class GFX2D : public olc::PGEX
	{
		// A representation of an affine transform, used to rotate, scale, offset & shear space
	public:
		class Transform2D
		{
		public:
			 Transform2D();

		public:
			// Set this transformation to unity
			 void Reset();
			// Append a rotation of fTheta radians to this transform
			 void Rotate(float fTheta);
			// Append a translation (ox, oy) to this transform
			 void Translate(float ox, float oy);
			// Append a scaling operation (sx, sy) to this transform
			 void Scale(float sx, float sy);
			// Append a shear operation (sx, sy) to this transform
			 void Shear(float sx, float sy);

			 void Perspective(float ox, float oy);
			// Calculate the Forward Transformation of the coordinate (in_x, in_y) -> (out_x, out_y)
			 void Forward(float in_x, float in_y, float &out_x, float &out_y);
			// Calculate the Inverse Transformation of the coordinate (in_x, in_y) -> (out_x, out_y)
			 void Backward(float in_x, float in_y, float &out_x, float &out_y);
			// Regenerate the Inverse Transformation
			 void Invert();

		private:
			 void Multiply();
			float matrix[4][3][3];
			int nTargetMatrix;
			int nSourceMatrix;
			bool bDirty;
		};

	public:
		// Draws a sprite with the transform applied
		 static void DrawSprite(olc::Sprite *sprite, olc::GFX2D::Transform2D &transform);
	};
}
