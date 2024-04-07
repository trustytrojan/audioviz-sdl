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
}

void Visualizer::start()
{
	PortAudio pa;
	auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);
	for (int frame = 0;; ++frame)
	{
		handleEvents();
		const auto h = window.GetHeight() - 10;
		const SDL2pp::Rect
			rect(5, 5, (window.GetWidth() - 10) / 2, h),
			rect2(rect.GetX() + rect.GetW() + 4, 5, (window.GetWidth() - 18) / 2, h);
		const auto start = std::chrono::high_resolution_clock::now();
		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
		if (!frames_read)
			return;
		try_write_audio_buffer(pa_stream);
		if (frames_read != sample_size)
			return;
		sr.SetDrawColor().Clear();
		// sr.SetDrawColor(50, 50, 50).FillRect(rect);
		// sr.SetDrawColor(100, 100, 100).FillRect(rect2);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
		sr.render_spectrum(rect, true);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);
		sr.render_spectrum(rect2, false);
		sr.Present();
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
		const auto draw_time = std::chrono::high_resolution_clock::now() - start;
		const auto fps = 1. / std::chrono::duration<double>(draw_time).count();
		if (!(frame % 5))
			print_render_stats(frame, draw_time, fps);
	}
}

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
				break;
			}
			break;
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
		// sr.SetDrawColor().Clear();
		// do_actual_rendering();
		// sr.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels, 3 * window.GetWidth());
		if (fwrite(pixels, 1, framesize, ffmpeg) < framesize)
			throw std::runtime_error(std::string("fwrite: ") + strerror(errno));
		sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
	}

	if (pclose(ffmpeg) == -1)
		throw std::runtime_error(std::string("pclose: ") + strerror(errno));
}
