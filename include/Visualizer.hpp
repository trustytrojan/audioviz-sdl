#pragma once

#include "MyRenderer.hpp"

class Visualizer
{
	bool running = true;
	int time = 0;

	void handleEvents();

public:
	SDL2pp::Window window;
	MyRenderer renderer;

	Visualizer();
	void start();
};