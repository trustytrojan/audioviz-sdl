#include <memory>
#include <argparse/argparse.hpp>
#include "FrequencySpectrum.hpp"
#include "Visualizer.hpp"

using argparse::ArgumentParser;

class Args : public ArgumentParser
{
	using ColorType = Visualizer::ColorType;
	using FS = FrequencySpectrum;

public:
	Args(const int argc, const char *const *const argv);
	std::unique_ptr<Visualizer> to_visualizer();
};