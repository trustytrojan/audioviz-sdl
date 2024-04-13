#include "Visualizer.hpp"
#include "ColorUtils.hpp"
#include <SDL2pp/SDLTTF.hh>

Visualizer::Visualizer(const std::string &audio_file, const int width, const int height)
	: audio_file(audio_file),
	  window("audioviz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE),
	  font_large("/usr/share/fonts/TTF/Iosevka-Regular.ttc", 24),
	  font_small("/usr/share/fonts/TTF/Iosevka-Regular.ttc", 18)
{
	font_large.SetStyle(TTF_STYLE_ITALIC);
	const SDL2pp::Color text_color{255, 255, 255, 180};
	if (const auto title = sf.getString(SF_STR_TITLE))
		texture_opts.title_text.emplace(sr, font_large.RenderUTF8_Blended(title, text_color));
	if (const auto artist = sf.getString(SF_STR_ARTIST))
		texture_opts.artist_text.emplace(sr, font_small.RenderUTF8_Blended(artist, text_color));
}

// assumes `texture_opts.bg` has a value
SDL2pp::Rect Visualizer::bg_texture_centered_max_width()
{
	const auto &texture = texture_opts.bg.value();
	const float aspect_ratio = (float)window.GetWidth() / window.GetHeight();

	// Calculate the height of the rectangle based on the renderer's aspect ratio
	int rectHeight = static_cast<int>(texture.GetWidth() / aspect_ratio);
	int rectWidth = texture.GetWidth();

	// If the calculated height is greater than the image height, adjust the width and height
	if (rectHeight > texture.GetHeight())
	{
		rectHeight = texture.GetHeight();
		rectWidth = static_cast<int>(texture.GetHeight() * aspect_ratio);
	}

	// Calculate the position of the rectangle to center it
	int rectX = (texture.GetWidth() - rectWidth) / 2;
	int rectY = (texture.GetHeight() - rectHeight) / 2;

	// Return the centered rectangle
	return SDL2pp::Rect(rectX, rectY, rectWidth, rectHeight);
}

void Visualizer::do_actual_rendering()
{
	const auto width = window.GetWidth(),
			   height = window.GetHeight();

	// still need to parameterize this
	static const auto margin = 5;

	sr.SetDrawColor().Clear();

	// default for stereo
	if (sf.channels() == 2 && mono < 0)
	{
		const auto w = (width - 2 * margin - sr.bar.get_spacing()) / 2;
		const auto h = height - 2 * margin;
		const SDL2pp::Rect
			rect1(margin, margin, w, h),
			rect2(rect1.x + rect1.w + sr.bar.get_spacing() - 1, margin, w, h);
		// uncomment to debug spectrum boundaries (which SpectrumRenderer should respect)
		// sr.SetDrawColor(255, 255, 255).DrawRect(rect1).DrawRect(rect2);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 0, true);
		sr.render_spectrum(rect1, true);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), 1, true);
		sr.render_spectrum(rect2, false);
	}
	// default for mono
	else
	{
		SDL2pp::Rect rect(margin, margin, width - 2 * margin, height - 2 * margin);
		sr.copy_channel_to_input(audio_buffer.data(), sf.channels(), mono, false);
		sr.render_spectrum(rect, false);
	}

	// apply shadow here (only if there is a background!)
	SDL_Texture *shadowed_texture = NULL;
	if (texture_opts.bg.has_value())
	{
		const auto surface = SDL_GetWindowSurface(window.Get());
		const auto pixels = static_cast<SDL_Color *>(surface->pixels);
		const auto num_pixels = width * height;
		for (int i = 0; i < num_pixels; ++i)
		{
			if (!pixels[i].a)
				continue;
			if (i >= width && !pixels[i - width].a)
				pixels[i - width].a = 100;
			if (i >= 2 * width && !pixels[i - 2 * width].a)
				pixels[i - 2 * width].a = 50;
		}
		shadowed_texture = SDL_CreateTextureFromSurface(sr.Get(), surface);
		SDL_SetTextureBlendMode(shadowed_texture, SDL_BLENDMODE_BLEND);
	}

	// bg
	if (texture_opts.bg.has_value())
		sr.Copy(texture_opts.bg.value(), bg_texture_centered_max_width());
	else
		sr.SetDrawColor().Clear();
	
	// copy spectrum texture with shadow over
	if (shadowed_texture)
		SDL_RenderCopy(sr.Get(), shadowed_texture, NULL, NULL);

	// metadata
	const SDL2pp::Point metadata_start{40, 40};

	const SDL2pp::Rect album_art_rect{metadata_start.x, metadata_start.y, 140, 140};
	const auto album_art_texture_present = texture_opts.album_art.has_value();
	if (album_art_texture_present)
		sr.Copy(texture_opts.album_art.value(), SDL2pp::NullOpt, album_art_rect);
	
	const SDL2pp::Point title_pt{metadata_start.x + album_art_texture_present * (album_art_rect.w + 10), album_art_rect.y};
	if (texture_opts.title_text.has_value())
		sr.Copy(texture_opts.title_text.value(), SDL2pp::NullOpt, title_pt);
	if (texture_opts.artist_text.has_value())
		sr.Copy(texture_opts.artist_text.value(), SDL2pp::NullOpt, {title_pt.x, title_pt.y + 30});
}

void Visualizer::start()
{
	// start portaudio stream for live audio playback
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
		if (frame % 10)
			return;
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
	const auto width = window.GetWidth(),
			   height = window.GetHeight();

	std::ostringstream ss;
	ss << '\'' << ffmpeg_path
	   << "' -hide_banner -y -f rawvideo -pix_fmt rgb24 -s:v "
	   << width << 'x' << height
	   << " -r " << fps
	   // this right here has solved the a/v desync!
	   // i'm sure this delay isn't going to be the right number for every machine
	   // but we're gonna roll with it
	   << " -i - -ss -0.1 -i '" << audio_file
	   << "' -c:v " << vcodec
	   << " -c:a " << acodec
	   << " -shortest '"
	   << output_file
	   << '\'';

	const auto command = ss.str();
	std::cout << command << '\n';
	const auto ffmpeg = popen(command.c_str(), "w");

	if (!ffmpeg)
		throw std::runtime_error(std::string("popen: ") + strerror(errno));

	const size_t framesize = 3 * width * height;
	Uint8 pixels[framesize];

	const auto afpvf = sf.samplerate() / fps;

	while (const auto frames_read = sf.readf(audio_buffer.data(), sample_size))
	{
		if (frames_read != sample_size)
			break;

		do_actual_rendering();

		// get pixels from renderer, send to ffmpeg
		sr.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels, 3 * window.GetWidth());
		if (fwrite(pixels, 1, framesize, ffmpeg) < framesize)
			throw std::runtime_error(std::string("fwrite: ") + strerror(errno));

		// seek audio backwards to synchronize with the audio file
		sf.seek(afpvf - sample_size, SEEK_CUR);
	}

	if (pclose(ffmpeg) == -1)
		throw std::runtime_error(std::string("pclose: ") + strerror(errno));
}
