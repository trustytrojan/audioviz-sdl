#include <iostream>

#include "../include/Visualizer.hpp"

int main()
{
	// Uncomment this to enforce Wayland instead of X11
	// setenv("SDL_VIDEODRIVER", "wayland", 1);

	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	Visualizer(800, 800).start();
}
