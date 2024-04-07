#include "Visualizer.hpp"
#include "ColorUtils.hpp"

Visualizer::Visualizer(const std::string &audio_file, const int width, const int height)
	: audio_file(audio_file),
	  window("My Audio Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE),
	  fs(sample_size)
{
	// get refresh rate
	SDL_DisplayMode mode;
	window.GetDisplayMode(mode);
	refresh_rate = mode.refresh_rate;
	resize_spectrum_array();
}

void Visualizer::resize_spectrum_array()
{
	if (stereo)
	{
		const auto mod4 = pill_count % 4;
		if (mod4)
			pill_count -= mod4;

		auto half_width = pill_count / 2;
		assert(!(half_width % 2));

		spectrum.resize(half_width);
	}
	else
	{
		spectrum.resize(pill_count);
	}
}

void Visualizer::start()
{
	start_sync();
}

void Visualizer::encode_to_video_popen(const std::string &output_file)
{
	const auto width = window.GetWidth(),
			   height = window.GetHeight();

	std::vector<std::string> args{
		"/bin/ffmpeg",
		"-hide_banner",
		"-y",

		// video
		"-f", "rawvideo",
		"-pix_fmt", "rgb24",
		"-s:v", std::to_string(width) + "x" + std::to_string(height),
		"-r", std::to_string(refresh_rate),
		"-i", "-",

		// audio
		"-i", audio_file,

		// end on shortest stream
		"-shortest",

		// output file
		output_file};

	std::ostringstream ss;
	for (size_t i = 0; i < args.size() - 1; ++i)
		ss << '\'' << args[i] << "' ";
	ss << args[args.size() - 1];

	const auto command = ss.str();
	const auto ffmpeg = popen(command.c_str(), "w");

	if (!ffmpeg)
		throw std::runtime_error(std::string("popen: ") + strerror(errno));

	const size_t framesize = 3 * width * height;
	Uint8 pixels[framesize];

	while (const auto frames_read = sf.readf(audio_buffer.data(), sample_size))
	{
		if (frames_read != sample_size)
			break;
		renderer.SetDrawColor().Clear();
		do_actual_rendering();
		renderer.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels, 3 * window.GetWidth());
		if (fwrite(pixels, 1, framesize, ffmpeg) < framesize)
			throw std::runtime_error(std::string("fwrite: ") + strerror(errno));
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
	}

	if (pclose(ffmpeg) == -1)
		throw std::runtime_error(std::string("pclose: ") + strerror(errno));
}

void Visualizer::start_sync()
{
	PortAudio pa;
	auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);
	for (int frame = 0;; ++frame)
	{
		handleEvents();
		const auto start = std::chrono::high_resolution_clock::now();
		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
		if (!frames_read)
			return;
		try_write_audio_buffer(pa_stream);
		if (frames_read != sample_size)
			return;
		renderer.SetDrawColor(0, 0, 0, 0).Clear();
		do_actual_rendering();
		renderer.Present();
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
		const auto draw_time = std::chrono::high_resolution_clock::now() - start;
		const auto fps = 1. / std::chrono::duration<double>(draw_time).count();
		if (!(frame % 5))
			print_render_stats(frame, draw_time, fps);
	}
}

void Visualizer::do_actual_rendering()
{
	if (stereo)
		for (int i = 1; i <= 2; ++i)
		{
			copy_channel_to_input(i);
			fs.render(spectrum);
			print_half(i);
		}
	else
	{
		copy_channel_to_input(1);
		fs.render(spectrum);
		print_full();
	}
}

// void Visualizer::prerender_frames()
// {
// 	for (int frame = 0;; ++frame)
// 	{
// 		const auto start = std::chrono::high_resolution_clock::now();
// 		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
// 		if (frames_read != sample_size)
// 			return;
// 		do_actual_rendering();
// 		spectrum_queue.push(spectrum);
// 		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
// 		const auto draw_time = std::chrono::high_resolution_clock::now() - start;
// 		const auto fps = 1. / std::chrono::duration<double>(draw_time).count();
// 		if (!(frame % 5))
// 			print_render_stats(frame, draw_time, fps);
// 	}
// }

void Visualizer::print_render_stats(const int frame, const std::chrono::nanoseconds draw_time, const double fps)
{
	std::cout << "\r\e[1A\e[2K\e[1A\e[2K"
			  << "Frame/Total: " << frame << '/' << total_frames << '(' << (((double)frame / total_frames) * 100) << "%)\n"
			  << "Draw time: " << draw_time << '\n'
			  << "FPS: " << fps;
}

void Visualizer::try_write_audio_buffer(PortAudio::Stream &pa_stream)
{
	try
	{
		pa_stream.write(audio_buffer.data(), audio_frames_per_video_frame());
	}
	catch (const PortAudio::Error &e)
	{
		if (!strstr(e.what(), "Output underflowed"))
			throw;
	}
}

void Visualizer::handleEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		switch (event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				pill_count = (window.GetWidth() - (2 * margin)) / (pill.width + pill.padding);
				resize_spectrum_array();
				break;
			}
			break;
		}
}

void Visualizer::copy_channel_to_input(const int channel_num)
{
	if (channel_num <= 0)
		throw std::invalid_argument("channel_num <= 0");
	if (channel_num > sf.channels())
		throw std::invalid_argument("channel_num > sf.channels()");
	const auto input = fs.input_array();
	for (int i = 0; i < sample_size; ++i)
		input[i] = audio_buffer[i * sf.channels() + channel_num];
}

void Visualizer::print_full()
{
	for (int i = 0; i < (int)spectrum.size(); ++i)
	{
		const auto [r, g, b] = apply_wheel_coloring(i, [this](const int i)
													{ return (float)i / spectrum.size(); });
		renderer.drawPillFromBottom(
			leftmostPillX() + (i * (pill.width + pill.padding)), pillY(), pill.width,
			multiplier * abs(spectrum[i]) * window.GetHeight(), r, g, b, 255);
	}
}

void Visualizer::print_half(int half)
{
	const int half_width = pill_count / 2;
	assert((int)spectrum.size() == half_width);
	assert(!(half_width % 2));
	switch (half)
	{
	case 1:
		for (int i = half_width - 1; i >= 0; --i)
		{
			const auto [r, g, b] = apply_wheel_coloring(i, [=](const int i)
														{ return (float)(half_width - i) / half_width; });
			drawPillAt(i, (half_width - 1) - i, r, g, b, 255);
		}
		break;

	case 2:
		for (int i = half_width; i < pill_count; ++i)
		{
			const auto [r, g, b] = apply_wheel_coloring(i, [=](const int i)
														{ return (float)i / half_width; });
			drawPillAt(i, i - half_width, r, g, b, 255);
		}
		break;

	default:
		throw std::logic_error("Visualizer::print_half: half can only be 1 or 2");
	}
}

void Visualizer::drawPillAt(const int pill_index, const int spectrum_index, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
{
	assert(pill_index >= 0);
	assert(pill_index < pill_count);
	const auto x = leftmostPillX() + (pill_index * (pill.width + pill.padding));
	const auto h = std::min(
		static_cast<float>(window.GetHeight() - margin),
		multiplier * std::max(0.f, spectrum[spectrum_index]) * window.GetHeight());
	renderer.drawPillFromBottom(x, pillY(), pill.width, h, r, g, b, a);
}

std::tuple<Uint8, Uint8, Uint8> Visualizer::apply_wheel_coloring(const int i, const std::function<float(int)> &ratio_calc)
{
	const auto [h, s, v] = wheel.hsv;
	return ColorUtils::hsvToRgb(ratio_calc(i) + h, s, v);
}