#include "Visualizer.hpp"
#include "ColorUtils.hpp"

Visualizer::Visualizer(const std::string &audio_file, const int width, const int height)
	: audio_file(audio_file),
	  window("audioviz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE) {}

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
			rect2(rect1.x + rect1.w + sr.bar.get_spacing() - 1, margin, w, h);
		// sr.SetDrawColor(255, 255, 255).DrawRect(rect1).DrawRect(rect2);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
		sr.render_spectrum(rect1, true);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);
		sr.render_spectrum(rect2, false);
	}
	// default for mono
	else
	{
		SDL2pp::Rect rect(margin, margin, window.GetWidth() - 2 * margin, window.GetHeight() - 2 * margin);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), mono, false);
		sr.render_spectrum(rect, false);
	}
}

void Visualizer::start()
{
	PortAudio pa;
	auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

	// calculate the number of audio frames that fit in each video frame
	SDL_DisplayMode mode;
	window.GetDisplayMode(mode);
	const auto avpvf = sf.samplerate() / mode.refresh_rate;
	const int total_frames = sf.frames() / avpvf;

	// for measuring render time
	using namespace std::chrono;
	using hrc = high_resolution_clock;
	hrc::time_point draw_start, fps_start;
	milliseconds draw_time;
	double fps;

	// frame counter
	int frame = 0;

	// lambda function to print render stats
	const auto print_render_stats = [&]
	{
		std::cout << "\r\e[2K\e[1A\e[2K\e[1A\e[2K"
				  << "Frame/Total: " << frame << '/' << total_frames << " (" << (((double)frame / total_frames) * 100) << "%)\n"
				  << "Draw time: " << draw_time << '\n'
				  << "FPS: " << fps
				  << std::flush;
	};

	for (; frame < total_frames; ++frame)
	{
		handle_events();

		// read in audio from file, write to portaudio stream
		const auto frames_read = sf.readf(audio_buffer.data(), sample_size);

		if (!frames_read)
			break;

		// occasionally, rendering might take too long until this is called again, causing "Output underflowed" to be thrown
		// the only way to get around this is dynamically changing the stream's samplerate to match our render time, but that sounds horrible...
		try
		{
			pa_stream.write(audio_buffer.data(), avpvf);
		}
		catch (const PortAudio::Error &e)
		{
			if (e.err != paOutputUnderflowed)
				throw;
		}

		if (frames_read != sample_size)
			break;

		// clear the screen
		sr.SetDrawColor().Clear();

		// perform rendering while measuring time
		draw_start = fps_start = hrc::now();
		do_actual_rendering();
		draw_time = duration_cast<milliseconds>(hrc::now() - draw_start);
		sr.Present();
		fps = 1 / duration<double>(hrc::now() - fps_start).count();
		print_render_stats();

		// seek audio file back
		sf.seek(avpvf - sample_size, SEEK_CUR);
	}

	print_render_stats();
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

void Visualizer::encode_to_video(const std::string &output_file, const int fps, const std::string &vcodec, const std::string &acodec)
{
	const auto afpvf = sf.samplerate() / fps;
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
		"-r", std::to_string(fps),
		"-i", "-",

		// audio
		"-i", audio_file,

		// copy audio stream (don't re-encode)
		"-c:v", vcodec,
		"-c:a", acodec,

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

		// no need to print render stats because ffmpeg shows that anyway
		do_actual_rendering();

		// get pixels from backbuffer, send to ffmpeg
		sr.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels, 3 * window.GetWidth());
		if (fwrite(pixels, 1, framesize, ffmpeg) < framesize)
			throw std::runtime_error(std::string("fwrite: ") + strerror(errno));

		// seek audio backwards to get next chunk to play
		sf.seek(afpvf - sample_size, SEEK_CUR);
	}

	if (pclose(ffmpeg) == -1)
		throw std::runtime_error(std::string("pclose: ") + strerror(errno));
}
