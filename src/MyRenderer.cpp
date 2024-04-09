#include "MyRenderer.hpp"

MyRenderer::MyRenderer(SDL2pp::Window &window, Uint32 flags)
	: SDL2pp::Renderer(window, -1, flags), _r(Get()) {}

void MyRenderer::drawBoxCentered(Uint16 x, Uint16 y, Uint16 w, Uint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	boxRGBA(_r, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2), r, g, b, a);
}

void MyRenderer::drawBoxFromBottomLeft(Uint16 x, Uint16 y, Uint16 w, Uint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	boxRGBA(_r, x, y - h, x + w, y, r, g, b, a);
}

void MyRenderer::drawCircleFromBottomLeft(Uint16 x, Uint16 y, Uint16 rad, bool filled, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	(filled ? filledCircleRGBA : circleRGBA)(_r, x + rad, y - rad, rad, r, g, b, a);
}

void MyRenderer::drawPillFromBottomLeft(Uint16 x, Uint16 y, Uint16 w, Uint16 h, bool filled, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	const auto rad = w / 2;
	drawCircleFromBottomLeft(x, y, rad, filled, r, g, b, a);
	// y -= rad;
	drawBoxFromBottomLeft(x, y - rad, w, h, r, g, b, a);
	// y -= h + rad;
	drawCircleFromBottomLeft(x, y - h, rad, filled, r, g, b, a);
}
