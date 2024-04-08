#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
	: Args(argc, argv), Visualizer(get("audio_file"), get<int>("--width"), get<int>("--height"))
{
	try
	{
		set_sample_size(get<int>("-n"));
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
		set_multiplier(get<float>("-m"));
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
		set_bar_width(get<int>("-bw"));
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
		set_bar_spacing(get<int>("-bs"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
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

	// { // spectrum coloring type
	// 	const auto &color_str = get("--color");
	// 	if (color_str == "wheel")
	// 	{
	// 		set_color_type(ColorType::WHEEL);
	// 		const auto &hsv_strs = get<std::vector<std::string>>("--hsv");
	// 		if (hsv_strs.size())
	// 			set_wheel_hsv({std::stof(hsv_strs[0]), std::stof(hsv_strs[1]), std::stof(hsv_strs[2])});
	// 		set_wheel_rate(get<float>("--wheel-rate"));
	// 	}
	// 	else if (color_str == "solid")
	// 	{
	// 		set_color_type(ColorType::SOLID);
	// 		const auto &rgb_strs = get<std::vector<std::string>>("--rgb");
	// 		if (rgb_strs.size())
	// 			set_solid_color({std::stoi(rgb_strs[0]), std::stoi(rgb_strs[1]), std::stoi(rgb_strs[2])});
	// 	}
	// 	else if (color_str == "none")
	// 		set_color_type(ColorType::NONE);
	// 	else
	// 		throw std::invalid_argument("unknown coloring type: " + color_str);
	// }

	{ // frequency scale (x-axis)
		const auto &scale_str = get("-s");
		if (scale_str == "linear")
			set_scale(FS::Scale::LINEAR);
		else if (scale_str == "log")
			set_scale(FS::Scale::LOG);
		else if (scale_str == "nth-root")
		{
			set_scale(FS::Scale::NTH_ROOT);
			const auto nth_root = get<float>("--nth-root");
			if (!nth_root)
				throw std::invalid_argument("nth_root cannot be zero!");
			set_nth_root(nth_root);
		}
		else
			throw std::invalid_argument("unknown scale: " + scale_str);
	}

	// start!!!!
	try
	{
		encode_to_video_popen(get("--encode"));
	}
	catch (std::invalid_argument &)
	{
		throw;
	}
	catch (std::logic_error &)
	{
		start();
	}
}