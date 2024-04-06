#include <memory>
#include <argparse/argparse.hpp>
#include "FrequencySpectrum.hpp"
#include "Visualizer.hpp"

using argparse::ArgumentParser;

class Args : public ArgumentParser
{
	using ColorType = Visualizer::ColorType;
	using Scale = FrequencySpectrum::Scale;
	using InterpType = FrequencySpectrum::InterpType;
	using AccumulationMethod = FrequencySpectrum::AccumulationMethod;
	using WindowFunction = FrequencySpectrum::WindowFunction;

public:
	Args(const int argc, const char *const *const argv);
	auto to_visualizer() -> std::unique_ptr<Visualizer>;
};