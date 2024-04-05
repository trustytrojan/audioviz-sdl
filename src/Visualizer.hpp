#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sndfile.hh>
#include "MyRenderer.hpp"
#include "FrequencySpectrum.hpp"
#include "PortAudio.hpp"
#include "ColorUtils.hpp"

class Visualizer
{
	int sample_size = 3000;
	float multiplier = 5;

	SDL2pp::Window window = SDL2pp::Window("My Audio Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);
	MyRenderer renderer = MyRenderer(window, (SDL_RendererFlags)(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));

	FrequencySpectrum fs = sample_size;

	SndfileHandle sf = "Music/bloodyeyes.mp3";
	const bool stereo = (sf.channels() == 2);

	PortAudio pa;
	PortAudio::Stream pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

	// intermediate arrays
	std::vector<float>
		timedata = std::vector<float>(sample_size),
		audio_buffer = std::vector<float>(sample_size * sf.channels()),
		spectrum = std::vector<float>((window.GetWidth() - 30) / 15); //(stereo ? (window.GetWidth() / 2) : tsize.width);

	const int refresh_rate = [this]()
	{
		SDL_DisplayMode mode;
		window.GetDisplayMode(mode);
		return mode.refresh_rate;
	}();

	const int audio_frames_per_video_frame = sf.samplerate() / refresh_rate;

	std::tuple<float, float, float> hsv{0.9, 0.7, 1};

public:
	void start()
	{
		for (int pos = 0; pos < sf.frames();)
		{
			handleEvents();

			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (frames_read != sample_size)
				return;

			// TODO:
			// this is going to need multithreading as sample_size increases
			// for example, if sample_size = 2000, we have to process 2000 samples,
			// while portaudio only has to play sf.samplerate() / refresh_rate samples,
			// which in the case of 44100Hz and 60Hz, is only 735 samples.
			// so fft is bound to take up more time than playback;
			// this is going to cause an "Output underflowed" error for sure.

			// either use threading, or consider buffering a certain amount of spectrums
			// before playing back audio.

			// render only when not minimized
			copy_channel_to_timedata(1);
			fs.render(timedata.data(), spectrum);
			renderer.SetDrawColor().Clear();
			render_spectrum_full();
			renderer.Present();

			try
			{
				pa_stream.write(audio_buffer.data(), audio_frames_per_video_frame);
			}
			catch (const PortAudio::Error &e)
			{
				if (!strstr(e.what(), "Output underflowed"))
					throw;
				std::cerr << "Output underflowed\n";
			}

			pos += audio_frames_per_video_frame;
			sf.seek(pos, SEEK_SET);
		}
	}

private:
	void handleEvents()
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
					spectrum.resize((window.GetWidth() - 30) / 15);
					break;
				}
				break;
			}
	}

	void copy_channel_to_timedata(const int channel_num)
	{
		if (channel_num <= 0)
			throw std::invalid_argument("channel_num <= 0");
		if (channel_num > sf.channels())
			throw std::invalid_argument("channel_num > sf.channels()");
		for (int i = 0; i < sample_size; ++i)
			timedata[i] = audio_buffer[i * sf.channels() + channel_num];
	}

	void render_spectrum_full()
	{
		const auto leftmostPillX = 15;
		const auto pillY = window.GetHeight() - 15;

		for (int i = 0; i < (int)spectrum.size(); ++i)
		{
			const auto [r, g, b] = apply_wheel_coloring(i, [this](auto i)
														{ return (float)i / spectrum.size(); });
			renderer.drawPillFromBottomRGBA(leftmostPillX + (i * 15), pillY, 10, multiplier * spectrum[i] * window.GetHeight(), r, g, b, 255);
		}
	}

	std::tuple<Uint8, Uint8, Uint8> apply_wheel_coloring(const int i, const std::function<float(int)> &ratio_calc)
	{
		const auto [h, s, v] = hsv;
		return ColorUtils::hsvToRgb(ratio_calc(i) + h, s, v);
	}
};