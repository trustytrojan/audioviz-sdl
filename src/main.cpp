#include <csignal>
#include "Main.hpp"

int main(const int argc, const char *const *const argv)
{
	signal(SIGINT, exit);
	setenv("SDL_VIDEODRIVER", "wayland", 1);
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	Main(argc, argv);
}