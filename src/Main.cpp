#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
	: Args(argc, argv), Visualizer(get("audio_file"), get<uint>("--width"), get<uint>("--height"))
{
	// all of these have default values, no need to try-catch
	set_sample_size(get<uint>("-n"));
	set_multiplier(get<float>("-m"));
	set_bar_width(get<uint>("-bw"));
	set_bar_spacing(get<uint>("-bs"));
	set_mono(get<int>("--mono"));

	try
	{
		set_background(get("--bg"));
	}
	catch (const std::logic_error &e)
	{
		if (!strstr(e.what(), "No value provided"))
			throw;
	}

	{ // bar type
		const auto &bt_str = get("-bt");
		if (bt_str == "bar")
			set_bar_type(SR::BarType::RECTANGLE);
		else if (bt_str == "pill")
			set_bar_type(SR::BarType::PILL);
		else
			throw std::invalid_argument("unknown bar type: " + bt_str);
	}

	{ // accumulation method
		const auto &am_str = get("-a");
		if (am_str == "sum")
			set_accum_method(FS::AccumulationMethod::SUM);
		else if (am_str == "max")
			set_accum_method(FS::AccumulationMethod::MAX);
		else
			throw std::invalid_argument("unknown accumulation method: " + am_str);
	}

	{ // window function
		const auto &wf_str = get("-w");
		if (wf_str == "hanning")
			set_window_function(FS::WindowFunction::HANNING);
		else if (wf_str == "hamming")
			set_window_function(FS::WindowFunction::HAMMING);
		else if (wf_str == "blackman")
			set_window_function(FS::WindowFunction::BLACKMAN);
		else if (wf_str == "none")
			set_window_function(FS::WindowFunction::NONE);
		else
			throw std::invalid_argument("unknown window function: " + wf_str);
	}

	{ // interpolation type
		const auto &interp_str = get("-i");
		if (interp_str == "none")
			set_interp_type(FS::InterpolationType::NONE);
		else if (interp_str == "linear")
			set_interp_type(FS::InterpolationType::LINEAR);
		else if (interp_str == "cspline")
			set_interp_type(FS::InterpolationType::CSPLINE);
		else if (interp_str == "cspline_hermite")
			set_interp_type(FS::InterpolationType::CSPLINE_HERMITE);
		else
			throw std::invalid_argument("unknown interpolation type: " + interp_str);
	}

	{ // spectrum coloring type
		const auto &color_str = get("--color");
		if (color_str == "wheel")
		{
			set_color_mode(SR::ColorMode::WHEEL);
			const auto &hsv = get<std::vector<float>>("--hsv");
			assert(hsv.size() == 3);
			set_color_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			set_color_wheel_rate(get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			set_color_mode(SR::ColorMode::SOLID);
			const auto &rgb = get<std::vector<Uint16>>("--rgb");
			set_color_solid_rgb({rgb[0], rgb[1], rgb[2]});
		}
		else
			throw std::invalid_argument("unknown coloring type: " + color_str);
	}

	// -s, --scale
	switch (const auto &scale_args = get<std::vector<std::string>>("-s"); scale_args.size())
	{
	case 0:
		break;
	case 1:
		if (scale_args[0] == "linear")
			set_scale(FS::Scale::LINEAR);
		else if (scale_args[0] == "log")
			set_scale(FS::Scale::LOG);
		else if (scale_args[0] == "nth-root")
			set_scale(FS::Scale::NTH_ROOT);
		break;
	case 2:
		if (scale_args[0] != "nth-root")
			throw std::invalid_argument("only the 'nth-root' scale takes an additional argument");
		set_nth_root(std::stoi(scale_args[1]));
	}

	// --encode (decides whether we render to the window or to a video)
	switch (const auto &encode_args = get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		start();
		break;
	case 2:
		encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()));
		break;
	case 3:
		encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()), encode_args[2]);
		break;
	case 4:
		encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()), encode_args[2], encode_args[3]);
		break;
	default:
		throw std::logic_error("--encode should only have 2-4 arguments");
	}
}