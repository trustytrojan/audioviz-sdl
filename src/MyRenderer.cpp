#include "../include/MyRenderer.hpp"

MyRenderer::MyRenderer(SDL2pp::Window &window, const SDL_RendererFlags flags)
	: SDL2pp::Renderer(window, -1, SDL_RENDERER_ACCELERATED | flags), r(Get()) {}

void MyRenderer::fillScreen(const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	SetDrawColor(r, g, b, a);
	Clear();
}

void MyRenderer::drawPill(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color)
{
	// (x, y) is the CENTER of the pill.

	// Draw the box centered about (x, y).
	boxColor(r, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2), color);

	// Draw the top circle at the top-middle of the box, aka (x, y - (h / 2))
	filledCircleColor(r, x, y - (h / 2), w / 2, color);

	// Draw the bottom circle at the bottom-middle of the box, aka (x, y + (h / 2))
	filledCircleColor(r, x, y + (h / 2), w / 2, color);
}

void MyRenderer::drawPill2(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint32 color)
{
	// (x, y) is the BOTTOM(-MIDDLE) of the pill.

	// Draw the box with the BOTTOM at (x, y):
	// Top-left corner:     x1 = x - (w / 2), y1 = y - h - (w / 2)
	// Bottom-right corner: x2 = x + (w / 2), y2 = y + (w / 2)
	boxColor(r, x - (w / 2), y - h, x + (w / 2), y, color);

	// Positioning the circles will be easy now.
	filledCircleColor(r, x, y - h, w / 2, color); // Top circle
	filledCircleColor(r, x, y, w / 2, color); // Bottom circle
}

void MyRenderer::drawArc(const Sint16 x, const Sint16 y, const Sint16 rad, const Sint16 start, const Sint16 end, const Uint32 color)
{
	arcColor(r, x, y, rad, start, end, color);
}
