#include "Args.hpp"

Args::Args(const int argc, const char *const *const argv)
	: ArgumentParser(argv[0])
{
	add_argument("audio_file")
		.help("audio file to visualize and play");

	add_argument("--encode")
		.help("encode to a video!");
	add_argument("--force-mono")
		.help("force a mono spectrum even if audio is stereo")
		.flag();

	add_argument("-n", "--sample-size")
		.help("number of samples (or frames of samples) to process at a time\n- higher -> increases accuracy\n- lower -> increases responsiveness")
		.default_value(3000)
		.scan<'i', int>()
		.validate();

	add_argument("-m", "--multiplier")
		.help("multiply spectrum amplitude by this amount")
		.default_value(4.f)
		.scan<'f', float>()
		.validate();

	add_argument("-s", "--scale")
		.help("spectrum frequency scale")
		.choices("linear", "log", "nth-root")
		.default_value("log")
		.validate();
	add_argument("--nth-root")
		.help("set the root to use with '--scale nth-root'")
		.default_value(2.f)
		.scan<'f', float>()
		.validate();

	add_argument("-a", "--accum-method")
		.help("frequency bin accumulation method\n- 'sum': greater treble detail, exaggerated amplitude\n- 'max': less treble detail, true-to-waveform amplitude")
		.choices("sum", "max")
		.default_value("max")
		.validate();

	add_argument("-w", "--window-func")
		.help("set window function to use, or 'none'.\nwindow functions can reduce 'wiggling' in bass frequencies.\nhowever they can reduce overall amplitude, so adjust '-m' accordingly.")
		.choices("none", "hanning", "hamming", "blackman")
		.default_value("blackman")
		.validate();

	add_argument("-i", "--interpolation")
		.help("spectrum interpolation type")
		.choices("none", "linear", "cspline", "cspline_hermite")
		.default_value("cspline")
		.validate();

	add_argument("--color")
		.help("enable a colorful spectrum!")
		.choices("wheel", "solid", "none")
		.default_value("wheel")
		.validate();
	add_argument("--wheel-rate")
		.help("requires '--color wheel'\nmoves the colors on the spectrum with time!\nvalue must be between [0, 1] - 0.005 is a good start")
		.default_value(0.f)
		.scan<'f', float>()
		.validate();
	add_argument("--hsv")
		.help("requires '--color wheel'\nchoose a hue offset for the color wheel, saturation, and brightness\nvalues must be between [0, 1]")
		.nargs(3)
		.validate();
	add_argument("--rgb")
		.help("requires '--color solid'\nrenders the spectrum with a solid color\nmust provide space-separated rgb integers")
		.nargs(3)
		.validate();

	add_argument("--width")
		.help("window width in pixels")
		.default_value(800)
		.scan<'i', int>()
		.validate();
	add_argument("--height")
		.help("window height in pixels")
		.default_value(600)
		.scan<'i', int>()
		.validate();
	
	add_argument("-bw", "--bar-width")
		.help("bar width in pixels")
		.default_value(10)
		.scan<'i', int>()
		.validate();
	add_argument("-bs", "--bar-spacing")
		.help("bar spacing in pixels")
		.default_value(5)
		.scan<'i', int>()
		.validate();
	add_argument("-bt", "--bar-type")
		.help("bar type")
		.choices("bar", "pill")
		.default_value("bar")
		.validate();

	try
	{
		parse_args(argc, argv);
	}
	catch (const std::exception &e)
	{
		// print error and help to stderr
		std::cerr << argv[0] << ": " << e.what() << '\n'
				  << *this;

		// just exit here since we don't want to print anything after the help
		_Exit(EXIT_FAILURE);
	}
}
