#include <SDL2pp/SDL2pp.hh>

namespace SDL2pp
{
	void PrintVideoDrivers(std::ostream &ostr = std::cerr);
	void PrintCurrentVideoDriver(std::ostream &ostr = std::cerr);
}