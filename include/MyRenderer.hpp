#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2pp/SDL2pp.hh>

// Extension of `SDL2pp::Renderer` to add convenience methods for calling `SDL2_gfx` functions.
class MyRenderer : public SDL2pp::Renderer
{
	SDL_Renderer *const _r;

public:
	MyRenderer(SDL2pp::Window &window, Uint32 flags);

	// (x, y) is the CENTER of the bar.
	void drawBarCentered(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	// (x, y) is the BOTTOM-LEFT of the bar.
	void drawBarFromBottom(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	// Draws a "pill" shape centered about (x, y), and with width `w` and height `h`.
	// At `h = 0`, the drawn shape is essentially a circle.
	void drawPillCentered(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	// Draws a "pill" shape with its bottom circle centered at (x, y), and with width `w` and height `h`.
	// At `h = 0`, the drawn shape is essentially a circle.
	void drawPillFromBottom(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

	void drawArc(Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
};
