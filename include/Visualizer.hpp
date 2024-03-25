#pragma once

#include "MyRenderer.hpp"

class Visualizer
{
	bool running = true;
	SDL2pp::Window window;
	MyRenderer renderer;

	void handleEvent(const SDL_Event &event);
	void handleEvents();

public:
	Visualizer(const int w, const int h);
	void start();
};