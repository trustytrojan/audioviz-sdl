#pragma once

#include "Visualizer.hpp"
#include "Args.hpp"

struct Main : Args, public Visualizer
{
	Main(const int argc, const char *const *const argv);
};