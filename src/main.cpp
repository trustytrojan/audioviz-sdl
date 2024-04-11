#include <csignal>
#include "Main.hpp"

int main(const int argc, const char *const *const argv)
{
	signal(SIGINT, exit);
	setenv("SDL_VIDEODRIVER", "wayland", 1);
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	try
	{
		Main(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << argv[0] << ": " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}