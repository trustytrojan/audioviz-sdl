#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2pp/SDL2pp.hh>

// Extension of `SDL2pp::Renderer` to add convenience methods for calling `SDL2_gfx` functions.
class MyRenderer : public SDL2pp::Renderer
{
	SDL_Renderer *const _r;

public:
	MyRenderer(SDL2pp::Window &window, const Uint32 flags);

	// Draws a "pill" shape centered about (x, y), and with width `w` and height `h`.
	// `color` should be a hexadecimal integer literal in the form: `0xRRGGBBAA`.
	void drawPillCentered(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color);

	// Draws a "pill" shape with the bottom at (x, y), and with width `w` and height `h`.
	// `color` should be a hexadecimal integer literal in the form: `0xRRGGBBAA`.
	void drawPillFromBottom(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color);

	void drawPillFromBottomRGBA(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a);

	void drawArc(const Sint16 x, const Sint16 y, const Sint16 rad, const Sint16 start, const Sint16 end, const Uint32 color);
};
