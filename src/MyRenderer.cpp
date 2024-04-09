#include "MyRenderer.hpp"

MyRenderer::MyRenderer(SDL2pp::Window &window, Uint32 flags)
	: SDL2pp::Renderer(window, -1, flags), _r(Get()) {}

void MyRenderer::drawBoxCentered(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (w <= 0 || h <= 0)
		throw std::invalid_argument("box dimensions must be positive integers!");
	boxRGBA(_r, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2), r, g, b, a);
}

void MyRenderer::drawBoxFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (w <= 0 || h <= 0)
		throw std::invalid_argument("box dimensions must be positive integers!");
	boxRGBA(_r, x, y - h, x + w - 1, y, r, g, b, a);
}

void MyRenderer::drawCircleFromBottomLeft(Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	if (rad >= 4)
		aacircleRGBA(_r, x + rad, y - rad, rad, r, g, b, a);
	filledCircleRGBA(_r, x + rad, y - rad, rad, r, g, b, a);
}

void MyRenderer::drawPillFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	auto rad = w / 2;
	drawCircleFromBottomLeft(x, y, rad, r, g, b, a);
	drawBoxFromBottomLeft(x, y - rad, w + !(w % 2), h, r, g, b, a);
	drawCircleFromBottomLeft(x, y - h + 1, rad, r, g, b, a);
}

void MyRenderer::drawCoolPillFromBottomLeft(Sint16 x, Sint16 y, Sint16 w, Sint16 h, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	static const auto shrink_factor = 2;
	if (w < 5*shrink_factor)
		throw std::invalid_argument("cool pill requires w >= " + std::to_string(5*shrink_factor));
	drawPillFromBottomLeft(x, y, w, h, 100, 100, 100, a);
	drawPillFromBottomLeft(x + shrink_factor, 	y - shrink_factor,	 w - 2*shrink_factor, h, r, g, b, a);
	drawPillFromBottomLeft(x + 2*shrink_factor, y - 2*shrink_factor, w - 4*shrink_factor, h, 100, 100, 100, a);
}