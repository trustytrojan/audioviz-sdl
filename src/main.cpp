#include <csignal>
#include "Args.hpp"

int main(const int argc, const char *const *const argv)
{
	signal(SIGINT, exit);
	setenv("SDL_VIDEODRIVER", "wayland", 1);
	SDL2pp::SDL sdl(SDL_INIT_VIDEO);
	Args args(argc, argv);
	const auto viz = args.to_visualizer();
	try
	{
		viz->encode_to_video_popen(args.get("--encode"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
		viz->start();
	}
}