#include "Visualizer.hpp"
#include "ColorUtils.hpp"

Visualizer::Visualizer(const std::string &audio_file, const int width, const int height)
	: audio_file(audio_file),
	  window("audioviz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE)
{
	// get refresh rate
	SDL_DisplayMode mode;
	window.GetDisplayMode(mode);
	refresh_rate = mode.refresh_rate;
}

void Visualizer::do_actual_rendering()
{
	static const auto margin = 5;
	// default for stereo
	if (sf.channels() == 2 && mono < 0)
	{
		const auto w = (window.GetWidth() - 2 * margin - sr.bar.get_spacing()) / 2;
		const auto h = window.GetHeight() - 2 * margin;
		SDL2pp::Rect
			rect1(margin, margin, w, h),
			rect2(rect1.x + rect1.w + sr.bar.get_spacing(), margin, w, h);
		sr.SetDrawColor(255, 255, 255).DrawRect(rect1).DrawRect(rect2);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
		sr.render_spectrum(rect1, true);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);
		sr.render_spectrum(rect2, false);
	}
	// default for mono
	else
	{
		SDL2pp::Rect rect(0, 0, window.GetWidth(), window.GetHeight());
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), mono, false);
		sr.render_spectrum(rect, false);
	}
}

void Visualizer::start()
{
	PortAudio pa;
	auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

	// for measuring render time
	std::chrono::_V2::system_clock::time_point start;

	int frame = 0;
	for (; frame < total_frames; ++frame)
	{
		handle_events();

		const auto measure_render_time = !(frame % 10);

		if (measure_render_time)
			start = std::chrono::high_resolution_clock::now();

		// read in audio from file, write to portaudio stream
		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
		if (!frames_read)
			break;
		try_write_audio_buffer(pa_stream);
		if (frames_read != sample_size)
			break;

		// clear the screen
		sr.SetDrawColor().Clear();

		do_actual_rendering();

		// present the rendered frame to the window
		sr.Present();

		// finish measuring render time and print
		if (measure_render_time)
			print_render_stats(frame, start);

		// seek audio file back
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
	}

	print_render_stats(frame, start);
}

void Visualizer::print_render_stats(const int frame, const std::chrono::_V2::system_clock::time_point &start)
{
	const auto draw_time = std::chrono::high_resolution_clock::now() - start;
	const auto fps = 1 / std::chrono::duration<double>(draw_time).count();
	std::cout << "\r\e[1A\e[2K\e[1A\e[2K"
			  << "Frame/Total: " << frame << '/' << total_frames << " (" << (((double)frame / total_frames) * 100) << "%)\n"
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

void Visualizer::handle_events()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
		switch (event.type)
		{
		case SDL_QUIT:
			exit(0);
		}
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

		// copy audio stream (don't re-encode)
		"-c:a", "copy",

		// end on shortest stream
		"-shortest",

		// output file
		output_file};

	std::ostringstream ss;
	for (size_t i = 0; i < args.size() - 1; ++i)
		ss << '\'' << args[i] << "' ";
	ss << '\'' << args[args.size() - 1] << '\'';

	const auto command = ss.str();
	std::cout << command << '\n';
	const auto ffmpeg = popen(command.c_str(), "w");

	if (!ffmpeg)
		throw std::runtime_error(std::string("popen: ") + strerror(errno));

	const size_t framesize = 3 * width * height;
	Uint8 pixels[framesize];

	while (const auto frames_read = sf.readf(audio_buffer.data(), sample_size))
	{
		if (frames_read != sample_size)
			break;

		// clear screen
		sr.SetDrawColor().Clear();

		do_actual_rendering();

		// get pixels from backbuffer, send to ffmpeg
		sr.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels, 3 * window.GetWidth());
		if (fwrite(pixels, 1, framesize, ffmpeg) < framesize)
			throw std::runtime_error(std::string("fwrite: ") + strerror(errno));

		// seek audio backwards to get next chunk to play
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
	}

	if (pclose(ffmpeg) == -1)
		throw std::runtime_error(std::string("pclose: ") + strerror(errno));
}
