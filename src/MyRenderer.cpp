#include "../include/MyRenderer.hpp"

MyRenderer::MyRenderer(SDL2pp::Window &window)
	: SDL2pp::Renderer(window, -1, SDL_RENDERER_ACCELERATED), r(Get()) {}

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

	// Draw the semi-circles at the ends
	filledCircleColor(r, x + h / 2, y + w / 2, w / 2, color);
	filledCircleColor(r, x + h / 2, y + h - w / 2, w / 2, color);
}

void MyRenderer::drawArc(const Sint16 x, const Sint16 y, const Sint16 rad, const Sint16 start, const Sint16 end, const Uint32 color)
{
	arcColor(r, x, y, rad, start, end, color);
}

void MyRenderer::drawStuff()
{
	SetDrawColor(100, 100, 100);
	DrawRect(20, 30, 60, 70);
	FillRect(500, 550, 600, 650);
	drawPill(250, 250, 10, 50, -1);
	drawArc(100, 100, 50, 0, 180, -1);
}

void MyRenderer::renderFrame()
{
	fillScreen();
	drawStuff();
	Present();
}
