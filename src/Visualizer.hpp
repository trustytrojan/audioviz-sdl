#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sndfile.hh>
#include <chrono>
#include "MyRenderer.hpp"
#include "FrequencySpectrum.hpp"
#include "PortAudio.hpp"
#include "ColorUtils.hpp"

class Visualizer
{
public:
	enum class ColorType
	{
		NONE,
		WHEEL,
		SOLID
	};

	using Scale = FrequencySpectrum::Scale;
	using InterpType = FrequencySpectrum::InterpType;
	using AccumulationMethod = FrequencySpectrum::AccumulationMethod;
	using WindowFunction = FrequencySpectrum::WindowFunction;

private:
	// general parameters
	int sample_size = 3000;
	float multiplier = 5;
	struct
	{
		int width = 10, padding = 5;
	} pill;
	int margin = 15;

	int spectrum_width()
	{
		return (window.GetWidth() - margin) / pill.width;
	}

	// SDL2pp window and renderer
	SDL2pp::Window window;
	MyRenderer renderer = MyRenderer(window, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// initialize spectrum renderer
	FrequencySpectrum fs = sample_size;

	// open audio file
	SndfileHandle sf;
	bool stereo = (sf.channels() == 2);

	// portaudio initialization
	PortAudio pa;
	PortAudio::Stream pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);

	// intermediate arrays
	std::vector<float>
		audio_buffer = std::vector<float>(sample_size * sf.channels()),
		spectrum = std::vector<float>(stereo ? (spectrum_width() / 2) : spectrum_width());

	// synchronization
	const int refresh_rate = [this]
	{
		SDL_DisplayMode mode;
		window.GetDisplayMode(mode);
		return mode.refresh_rate;
	}();
	const int audio_frames_per_video_frame = sf.samplerate() / refresh_rate;

	// color stuff
	ColorType color_type = ColorType::WHEEL;
	std::tuple<int, int, int> solid_rgb{255, 0, 255};

	// color wheel rotation
	struct
	{
		float time = 0, rate = 0;
		std::tuple<float, float, float> hsv{0.9, 0.7, 1};
	} wheel;

	// for pre-rendering
	bool prerender = false;
	std::queue<std::vector<float>> spectrum_queue;

public:
	// width and height matter if you are pre-rendering!
	Visualizer(const std::string &audio_file, const int width = 800, const int height = 600)
		: sf(audio_file),
		  window("My Audio Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE) {}

	void start()
	{
		std::cout << refresh_rate << '\n';
		std::cout << audio_frames_per_video_frame << '\n';

		if (prerender)
		{
			prerender_frames();
			start_prerendered();
		}
		else
			start_sync();
	}

	/**
	 * Set the sample chunk size to use in internal calculations.
	 * @note Smaller values increase responsiveness, but decrease accuracy. Larger values do the opposite.
	 * @note This method is thread safe.
	 * @param sample_size new sample size to use
	 * @return reference to self
	 */
	Visualizer &set_sample_size(const int sample_size)
	{
		this->sample_size = sample_size;
		fs.set_fft_size(sample_size);
		audio_buffer.resize(sample_size * sf.channels());
		return *this;
	}

	/**
	 * Set the multiplier to multiply the spectrum's height by.
	 * @param multiplier new multiplier to use
	 * @return reference to self
	 */
	Visualizer &set_multiplier(const float multiplier)
	{
		this->multiplier = multiplier;
		return *this;
	}

	Visualizer &set_prerender(bool prerender)
	{
		this->prerender = prerender;
		return *this;
	}

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	Visualizer &set_interp_type(const InterpType interp_type)
	{
		fs.set_interp_type(interp_type);
		return *this;
	}

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	Visualizer &set_scale(const Scale scale)
	{
		fs.set_scale(scale);
		return *this;
	}

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	Visualizer &set_nth_root(const int nth_root)
	{
		fs.set_nth_root(nth_root);
		return *this;
	}

	/**
	 * Set frequency bin accumulation method.
	 * @note Choosing `SUM` results in more visible detail in the treble frequencies, at the cost of their amplitudes being visually exaggerated.
	 * @note Choosing `MAX` results in a more true-to-waveform frequency distribution, however treble frequencies aren't very visible.
	 * @param interp new accumulation method to use
	 * @returns reference to self
	 */
	Visualizer &set_accum_method(const AccumulationMethod method)
	{
		fs.set_accum_method(method);
		return *this;
	}

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	Visualizer &set_window_function(const WindowFunction wf)
	{
		fs.set_window_func(wf);
		return *this;
	}

private:
	const int total_frames = sf.frames() / audio_frames_per_video_frame;

	void start_sync()
	{
		for (int frame = 0;; ++frame)
		{
			handleEvents();
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (!frames_read)
				return;
			try_write_audio_buffer();
			if (frames_read != sample_size)
				return;
			std::cout << "Rendering frame: " << frame << '/' << total_frames << '\r' << std::flush;
			renderer.SetDrawColor().Clear();
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
				render_spectrum_full();
			}
			renderer.Present();
			sf.seek(audio_frames_per_video_frame, SEEK_CUR);
		}
	}

	void prerender_frames()
	{
		for (int frame = 0;; ++frame)
		{
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			copy_channel_to_input(1);
			fs.render(spectrum);
			spectrum_queue.push(spectrum);
			std::cout << "Pre-rendering frame: " << frame << '/' << total_frames << '\r' << std::flush;
			sf.seek(audio_frames_per_video_frame, SEEK_CUR);
		}
		std::cout << '\n';
	}

	void start_prerendered()
	{
		sf.seek(0, SEEK_SET);
		for (int frame = 0; !spectrum_queue.empty(); ++frame)
		{
			handleEvents();
			const auto frames_read = sf.readf(audio_buffer.data(), audio_frames_per_video_frame);
			if (!frames_read)
				return;
			try_write_audio_buffer();
			this->spectrum = spectrum_queue.front();
			spectrum_queue.pop();
			std::cout << "Presenting frame: " << frame << '/' << total_frames << '\r' << std::flush;
			clear_render_present();
		}
	}

	void clear_render_present()
	{
		renderer.SetDrawColor().Clear();
		render_spectrum_full();
		renderer.Present();
	}

	void try_write_audio_buffer()
	{
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
	}

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

	void copy_channel_to_input(const int channel_num)
	{
		if (channel_num <= 0)
			throw std::invalid_argument("channel_num <= 0");
		if (channel_num > sf.channels())
			throw std::invalid_argument("channel_num > sf.channels()");
		const auto input = fs.input_array();
		for (int i = 0; i < sample_size; ++i)
			input[i] = audio_buffer[i * sf.channels() + channel_num];
	}

	void render_spectrum_full()
	{
		const auto leftmostPillX = margin;
		const auto pillY = window.GetHeight() - margin;

		for (int i = 0; i < (int)spectrum.size(); ++i)
		{
			const auto [r, g, b] = apply_wheel_coloring(i, [this](auto i)
														{ return (float)i / spectrum.size(); });
			renderer.drawPillFromBottomRGBA(
				leftmostPillX + (i * (pill.width + pill.padding)), pillY, pill.width,
				multiplier * abs(spectrum[i]) * window.GetHeight(), r, g, b, 255);
		}
	}

	void print_half(int half)
	{
		// const auto half_width = spectrum_width() / 2;
		const auto leftmostPillX = margin;
		const auto pillY = window.GetHeight() - margin;

		if (half == 1)
			for (int i = spectrum_width(); i >= 0; --i)
			{
				const auto [r, g, b] = apply_wheel_coloring(i, [this](auto i)
															{ return (float)(spectrum_width() - i) / spectrum.size(); });
				// move_to_column(i);
				// print_bar(multiplier * spectrum[half_width - i] * tsize.height);
				renderer.drawPillFromBottomRGBA(
					leftmostPillX + (i * (pill.width + pill.padding)), pillY, pill.width,
					multiplier * abs(spectrum[i]) * window.GetHeight(), r, g, b, 255);
			}

		else if (half == 2)
			for (int i = half_width; i < tsize.width; ++i)
			{
				if (color_type == ColorType::WHEEL)
					apply_wheel_coloring(i, [half_width](const int i)
										 { return (float)i / half_width; });
				move_to_column(i);
				print_bar(multiplier * spectrum[i - half_width] * tsize.height);
			}
	}

	std::tuple<Uint8, Uint8, Uint8> apply_wheel_coloring(const int i, const std::function<float(int)> &ratio_calc)
	{
		const auto [h, s, v] = wheel.hsv;
		return ColorUtils::hsvToRgb(ratio_calc(i) + h, s, v);
	}
};