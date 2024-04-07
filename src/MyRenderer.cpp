#include "MyRenderer.hpp"

MyRenderer::MyRenderer(SDL2pp::Window &window, const Uint32 flags)
	: SDL2pp::Renderer(window, -1, flags), _r(Get()) {}

void MyRenderer::drawBarCentered(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	boxRGBA(_r, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2), r, g, b, a);
}

void MyRenderer::drawBarFromBottom(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	boxRGBA(_r, x, y - h, x + w, y, r, g, b, a);
}

void MyRenderer::drawPillCentered(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	drawBarCentered(x, y, w, h, r, g, b, a);

	// Draw the top circle at the top-middle of the box, aka (x, y - (h / 2))
	filledCircleRGBA(_r, x, y - (h / 2), w / 2, r, g, b, a);

	// Draw the bottom circle at the bottom-middle of the box, aka (x, y + (h / 2))
	filledCircleRGBA(_r, x, y + (h / 2), w / 2, r, g, b, a);
}

void MyRenderer::drawPillFromBottom(const Sint16 x, const Sint16 y, const Sint16 w, const Sint16 h, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	// Luckily we already have a method to draw a rectangle/box/bar from it's bottom.
	drawBarFromBottom(x, y, w, h, r, g, b, a);

	// Positioning the circles will be easy now.
	// If you need to understand this, grab a paper and pencil.
	filledCircleRGBA(_r, x, y - h, w / 2, r, g, b, a);
	filledCircleRGBA(_r, x, y, w / 2, r, g, b, a);
}

void MyRenderer::drawArc(const Sint16 x, const Sint16 y, const Sint16 rad, const Sint16 start, const Sint16 end, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	arcRGBA(_r, x, y, rad, start, end, r, g, b, a);
}