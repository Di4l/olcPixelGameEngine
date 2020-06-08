//-----------------------------------------------------------------------------
#include "pgeDecal.h"
#include "pgeCore.h"
//-----------------------------------------------------------------------------
using namespace olc;
//-----------------------------------------------------------------------------

Decal::Decal(olc::ISprite* spr)
{
	id = -1;
	if (spr == nullptr) return;
	sprite = spr;
	id = renderer->CreateTexture(sprite->width, sprite->height);
	Update();
}
//-----------------------------------------------------------------------------

void Decal::Update()
{
	if (sprite == nullptr) return;
	vUVScale = { 1.0f / float(sprite->width), 1.0f / float(sprite->height) };
	renderer->ApplyTexture(id);
	renderer->UpdateTexture(id, sprite);
}
//-----------------------------------------------------------------------------

Decal::~Decal()
{
	if (id != -1)
	{
		renderer->DeleteTexture(id);
		id = -1;
	}
}
//-----------------------------------------------------------------------------
