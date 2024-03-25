#include "../include/Visualizer.hpp"

Visualizer::Visualizer()
	: window("My Audio Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0),
	  renderer(window, SDL_RENDERER_PRESENTVSYNC) {}

void Visualizer::handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;
		}
}

void Visualizer::start()
{
	while (running)
	{
		handleEvents();

		// Fill the screen with black
		renderer.SetDrawColor().Clear();

		if (time > 500)
			time = 0;

		const auto rightmostPillX = window.GetWidth() - 50;
		const auto pillY = window.GetHeight() - 50;

		// Draw a white pill anchored 50,50 away from the bottom right corner,
		// then several more 5px to the left of each other.
		for (auto i = 0; i < 450; i += 15) // 15 accomodates for the 10px pill width, and adds 5px margin between pills
			renderer.drawPill2(rightmostPillX - i, pillY, 10, time + i, 0xffffffff);

		renderer.Present();

		++time;
	}
}
