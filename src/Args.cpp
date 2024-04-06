#include "Args.hpp"

Args::Args(const int argc, const char *const *const argv)
	: ArgumentParser(argv[0])
{
	add_argument("audio_file")
		.help("audio file to visualize and play");

	// add_argument("--prerender")
	// 	.help("prerender all spectrum frames before playback to avoid output underflow")
	// 	.flag();
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

	add_argument("-a", "--accum")
		.help("frequency bin accumulation method\n- 'sum': greater treble detail, exaggerated amplitude\n- 'max': less treble detail, true-to-waveform amplitude")
		.choices("sum", "max")
		.nargs(1)
		.default_value("max")
		.validate();

	add_argument("-w", "--window")
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

	add_argument("--pw", "--pill-width")
		.help("pill width in pixels")
		.scan<'i', int>()
		.validate();
	add_argument("--pp", "--pill-padding")
		.help("padding between pills in pixels")
		.scan<'i', int>()
		.validate();
	add_argument("--margin")
		.help("margin in pixels between window edge and spectrum pills")
		.scan<'i', int>()
		.validate();

	add_argument("--width")
		.help("window width in pixels")
		.scan<'i', int>()
		.validate();
	add_argument("--height")
		.help("window height in pixels")
		.scan<'i', int>()
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

auto Args::to_visualizer() -> std::unique_ptr<Visualizer>
{
	std::unique_ptr<Visualizer> viz(new Visualizer(get("audio_file")));

	try
	{
		viz->set_sample_size(get<int>("-n"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	try
	{
		viz->set_multiplier(get<float>("-m"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	try
	{
		viz->set_width(get<int>("--width"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	try
	{
		viz->set_height(get<int>("--height"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	// viz->set_prerender(get<bool>("--prerender"));
	if (get<bool>("--force-mono"))
		viz->set_stereo(false);

	try
	{
		viz->set_pill_width(get<int>("--pill-width"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	try
	{
		viz->set_pill_padding(get<int>("--pill-padding"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	try
	{
		viz->set_margin(get<int>("--margin"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
	}

	{ // accumulation method
		const auto &acc_method_str = get("-a");
		if (acc_method_str == "sum")
			viz->set_accum_method(AccumulationMethod::SUM);
		else if (acc_method_str == "max")
			viz->set_accum_method(AccumulationMethod::MAX);
		else
			throw std::invalid_argument("unknown accumulation method: " + acc_method_str);
	}

	{ // window function
		const auto &wfunc_str = get("-w");
		if (wfunc_str == "hanning")
			viz->set_window_function(WindowFunction::HANNING);
		else if (wfunc_str == "hamming")
			viz->set_window_function(WindowFunction::HAMMING);
		else if (wfunc_str == "blackman")
			viz->set_window_function(WindowFunction::BLACKMAN);
		else if (wfunc_str == "none")
			viz->set_window_function(WindowFunction::NONE);
		else
			throw std::invalid_argument("unknown window function: " + wfunc_str);
	}

	{ // interpolation type
		const auto &interp_str = get("-i");
		if (interp_str == "none")
			viz->set_interp_type(InterpType::NONE);
		else if (interp_str == "linear")
			viz->set_interp_type(InterpType::LINEAR);
		else if (interp_str == "cspline")
			viz->set_interp_type(InterpType::CSPLINE);
		else if (interp_str == "cspline_hermite")
			viz->set_interp_type(InterpType::CSPLINE_HERMITE);
		else
			throw std::invalid_argument("unknown interpolation type: " + interp_str);
	}

	// { // spectrum coloring type
	// 	const auto &color_str = get("--color");
	// 	if (color_str == "wheel")
	// 	{
	// 		viz->set_color_type(ColorType::WHEEL);
	// 		const auto &hsv_strs = get<std::vector<std::string>>("--hsv");
	// 		if (hsv_strs.size())
	// 			viz->set_wheel_hsv({std::stof(hsv_strs[0]), std::stof(hsv_strs[1]), std::stof(hsv_strs[2])});
	// 		viz->set_wheel_rate(get<float>("--wheel-rate"));
	// 	}
	// 	else if (color_str == "solid")
	// 	{
	// 		viz->set_color_type(ColorType::SOLID);
	// 		const auto &rgb_strs = get<std::vector<std::string>>("--rgb");
	// 		if (rgb_strs.size())
	// 			viz->set_solid_color({std::stoi(rgb_strs[0]), std::stoi(rgb_strs[1]), std::stoi(rgb_strs[2])});
	// 	}
	// 	else if (color_str == "none")
	// 		viz->set_color_type(ColorType::NONE);
	// 	else
	// 		throw std::invalid_argument("unknown coloring type: " + color_str);
	// }

	{ // frequency scale (x-axis)
		const auto &scale_str = get("-s");
		if (scale_str == "linear")
			viz->set_scale(Scale::LINEAR);
		else if (scale_str == "log")
			viz->set_scale(Scale::LOG);
		else if (scale_str == "nth-root")
		{
			viz->set_scale(Scale::NTH_ROOT);
			const auto nth_root = get<float>("--nth-root");
			if (!nth_root)
				throw std::invalid_argument("nth_root cannot be zero!");
			viz->set_nth_root(nth_root);
		}
		else
			throw std::invalid_argument("unknown scale: " + scale_str);
	}

	return viz;
}