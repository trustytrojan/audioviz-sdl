#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2pp/SDL2pp.hh>

// Extension of `SDL2pp::Renderer` to add convenience methods for calling `SDL2_gfx` functions.
class MyRenderer : public SDL2pp::Renderer
{
	SDL_Renderer *const _r;

public:
	MyRenderer(SDL2pp::Window &window, Uint32 flags);

	// (x, y) is the CENTER of the box (filled-color rectangle).
	void drawBoxCentered(Uint16 x, Uint16 y, Uint16 w, Uint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM-LEFT of the box (filled-color rectangle).
	void drawBoxFromBottomLeft(Uint16 x, Uint16 y, Uint16 w, Uint16 h, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM of the circle.
	void drawCircleFromBottomLeft(Uint16 x, Uint16 y, Uint16 rad, bool filled = true, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);

	// (x, y) is the BOTTOM of the pill (rounded-rectangle with `x1 == x2`).
	void drawPillFromBottomLeft(Uint16 x, Uint16 y, Uint16 w, Uint16 h, bool filled = true, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
};
