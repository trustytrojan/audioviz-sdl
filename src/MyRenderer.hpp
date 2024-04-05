#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2pp/SDL2pp.hh>

// Extension of `SDL2pp::Renderer` to add convenience methods for calling `SDL2_gfx` functions.

class MyRenderer : public SDL2pp::Renderer
{
	SDL_Renderer *const _r;

public:
	MyRenderer(SDL2pp::Window &window, const SDL_RendererFlags flags)
		: SDL2pp::Renderer(window, -1, SDL_RENDERER_ACCELERATED | flags), _r(Get()) {}

	void fillScreen(const Uint8 r = 0, const Uint8 g = 0, const Uint8 b = 0, const Uint8 a = 255)
	{
		SetDrawColor(r, g, b, a);
		Clear();
	}

	// Draws a "pill" shape centered about (x, y), and with width `w` and height `h`.
	// `color` should be a hexadecimal integer literal in the form: `0xRRGGBBAA`.
	void drawPillCentered(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color)
	{
		// (x, y) is the CENTER of the pill.

		// Draw the box centered about (x, y).
		boxColor(_r, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2), color);

		// Draw the top circle at the top-middle of the box, aka (x, y - (h / 2))
		filledCircleColor(_r, x, y - (h / 2), w / 2, color);

		// Draw the bottom circle at the bottom-middle of the box, aka (x, y + (h / 2))
		filledCircleColor(_r, x, y + (h / 2), w / 2, color);
	}

	// Draws a "pill" shape with the bottom at (x, y), and with width `w` and height `h`.
	// `color` should be a hexadecimal integer literal in the form: `0xRRGGBBAA`.
	void drawPillFromBottom(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color)
	{
		// (x, y) is the BOTTOM(-MIDDLE) of the pill.

		// Draw the box with the BOTTOM at (x, y):
		// Top-left corner:     x1 = x - (w / 2), y1 = y - h - (w / 2)
		// Bottom-right corner: x2 = x + (w / 2), y2 = y + (w / 2)
		boxColor(_r, x - (w / 2), y - h, x + (w / 2), y, color);

		// Positioning the circles will be easy now.
		filledCircleColor(_r, x, y - h, w / 2, color); // Top circle
		filledCircleColor(_r, x, y, w / 2, color);	  // Bottom circle
	}

	void drawPillFromBottomRGBA(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
	{
		boxRGBA(_r, x - (w / 2), y - h, x + (w / 2), y, r, g, b, a);
		filledCircleRGBA(_r, x, y - h, w / 2, r, g, b, a);
		filledCircleRGBA(_r, x, y, w / 2, r, g, b, a);
	}

	void drawArc(const Sint16 x, const Sint16 y, const Sint16 rad, const Sint16 start, const Sint16 end, const Uint32 color)
	{
		arcColor(_r, x, y, rad, start, end, color);
	}
};
