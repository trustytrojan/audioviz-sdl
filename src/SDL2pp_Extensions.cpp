#include "../include/SDL2pp_Extensions.hpp"

void SDL2pp::PrintVideoDrivers(std::ostream &ostr)
{
	const auto numVideoDrivers = SDL_GetNumVideoDrivers();
	if (numVideoDrivers <= 0)
		throw SDL2pp::Exception("SDL_GetNumVideoDrivers");
	for (auto i = 0; i < numVideoDrivers - 1; ++i)
		ostr << SDL_GetVideoDriver(i) << ", ";
	ostr << SDL_GetVideoDriver(numVideoDrivers - 1) << '\n';
}

void SDL2pp::PrintCurrentVideoDriver(std::ostream &ostr)
{
	const auto currentVideoDriver = SDL_GetCurrentVideoDriver();
	if (!currentVideoDriver)
		ostr << "no video driver initialized\n";
	ostr << currentVideoDriver << '\n';
}
