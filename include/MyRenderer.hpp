#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2pp/SDL2pp.hh>

// Extension of `SDL2pp::Renderer` to add convenience methods for calling `SDL2_gfx` functions.
class MyRenderer : public SDL2pp::Renderer
{
protected:
	SDL_Renderer *const _r;

public:
	MyRenderer(SDL2pp::Window &window, Uint32 flags);

	// (x, y) is the CENTER of the box (filled-color rectangle).
	void drawBoxCentered(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM-LEFT of the box (filled-color rectangle).
	void drawBoxFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM of the circle.
	void drawCircleFromBottomLeft(Sint16 x, Sint16 y, Sint16 rad, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM of the pill (rounded-rectangle with `x1 == x2`).
	void drawPillFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	void drawCoolPillFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
};
