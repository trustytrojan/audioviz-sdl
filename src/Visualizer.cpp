#include "../include/Visualizer.hpp"

Visualizer::Visualizer(const int w, const int h)
	: window("SDL2pp window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0),
	  renderer(window) {}

void Visualizer::handleEvent(const SDL_Event &event)
{
	switch (event.type)
	{
	case SDL_QUIT:
		running = false;
	}
}

void Visualizer::handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		handleEvent(event);
}

void Visualizer::start()
{
	while (running)
	{
		handleEvents();
		renderer.renderFrame();
	}
}
