#include <iostream>
#include <csignal>
#include "Args.hpp"

int main(const int argc, const char *const *const argv)
{
	std::signal(SIGINT, exit);
	setenv("SDL_VIDEODRIVER", "wayland", 1);
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	Args(argc, argv).to_visualizer()
		->start();
}