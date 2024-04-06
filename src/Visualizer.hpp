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
// #include "VideoEncoder.hpp"

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

	// width and height matter if you are pre-rendering!
	Visualizer(const std::string &audio_file, const int width = 800, const int height = 600)
		: window("My Audio Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE),
		  sf(audio_file) {}

	void start()
	{
		std::cout << refresh_rate << '\n';
		std::cout << audio_frames_per_video_frame() << '\n';

		if (prerender)
		{
			prerender_frames();
			start_prerendered();
			return;
		}

		start_sync();
	}

	void encode_to_video(const std::string &output_file)
	{
		const auto width = window.GetWidth(),
				   height = window.GetHeight();
		const auto pixel_count = width * height;

		const auto command = "ffmpeg -y "
							 "-f rawvideo "
							 "-pix_fmt rgb24 "
							 "-s:v " +
							 std::to_string(width) + "x" + std::to_string(height) + " "
																					"-r " +
							 std::to_string(refresh_rate) + " "
															"-i - "
															"-i 'Music/obsessed (feat. funeral).mp3' "
															"-c:a copy " +
							 output_file;
		const auto ffmpeg = popen(command.c_str(), "w");
		if (!ffmpeg)
			throw std::runtime_error(std::string("popen() failed") + strerror(errno));

		std::vector<Uint8> pixels(3 * pixel_count);
		for (int frame = 0;; ++frame)
		{
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (frames_read != sample_size)
				return;
			renderer.SetDrawColor().Clear();
			do_actual_rendering();
			renderer.ReadPixels(SDL2pp::NullOpt, SDL_PIXELFORMAT_RGB24, pixels.data(), 3 * window.GetWidth());
			fwrite(pixels.data(), 1, 3 * pixel_count, ffmpeg);
			sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
		}
	}

	Visualizer &set_width(const int width)
	{
		window.SetSize(width, window.GetHeight());
		return *this;
	}

	Visualizer &set_height(const int height)
	{
		window.SetSize(window.GetWidth(), height);
		return *this;
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

	Visualizer &set_stereo(bool enabled)
	{
		stereo = enabled;
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

	Visualizer &set_pill_width(const int width)
	{
		pill.width = width;
		return *this;
	}

	Visualizer &set_pill_padding(const int padding)
	{
		pill.padding = padding;
		return *this;
	}

	Visualizer &set_margin(const int margin)
	{
		this->margin = margin;
		return *this;
	}

private:
	// general parameters
	int sample_size = 3000;
	float multiplier = 5;

	struct
	{
		int width = 10, padding = 5;
	} pill;

	int margin = 15;

	int pill_count() const
	{
		return (window.GetWidth() - (2 * margin)) / (pill.width + pill.padding);
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
		spectrum = std::vector<float>(stereo ? (pill_count() / 2) : pill_count());

	// synchronization
	int refresh_rate = [this]
	{
		SDL_DisplayMode mode;
		window.GetDisplayMode(mode);
		return mode.refresh_rate;
	}();

	int audio_frames_per_video_frame() const { return sf.samplerate() / refresh_rate; }

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

	const int total_frames = sf.frames() / audio_frames_per_video_frame();

	void start_sync()
	{
		for (int frame = 0;; ++frame)
		{
			handleEvents();
			const auto start = std::chrono::high_resolution_clock::now();
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (!frames_read)
				return;
			try_write_audio_buffer();
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

	void do_actual_rendering()
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
			render_spectrum_full();
		}
	}

	void prerender_frames()
	{
		for (int frame = 0;; ++frame)
		{
			const auto start = std::chrono::high_resolution_clock::now();
			const auto frames_read = sf.readf(audio_buffer.data(), sample_size);
			if (frames_read != sample_size)
				return;
			do_actual_rendering();
			spectrum_queue.push(spectrum);
			sf.seek(audio_frames_per_video_frame() - sample_size, SEEK_CUR);
			const auto draw_time = std::chrono::high_resolution_clock::now() - start;
			const auto fps = 1. / std::chrono::duration<double>(draw_time).count();
			if (!(frame % 5))
				print_render_stats(frame, draw_time, fps);
		}
	}

	void print_render_stats(const int frame, const std::chrono::nanoseconds draw_time, const double fps)
	{
		std::cout << "\r\e[1A\e[2K\e[1A\e[2K"
				  << "Frame/Total: " << frame << '/' << total_frames << '\n'
				  << "Draw time: " << draw_time << '\n'
				  << "FPS: " << fps;
	}

	void start_prerendered()
	{
		sf.seek(0, SEEK_SET);
		for (int frame = 0; !spectrum_queue.empty(); ++frame)
		{
			handleEvents();
			const auto frames_read = sf.readf(audio_buffer.data(), audio_frames_per_video_frame());
			if (!frames_read)
				return;
			try_write_audio_buffer();
			this->spectrum = spectrum_queue.front();
			spectrum_queue.pop();
			std::cout << "Presenting frame: " << frame << '/' << total_frames << '\r' << std::flush;
			renderer.SetDrawColor(0, 0, 0, 0).Clear();
			do_actual_rendering();
			renderer.Present();
		}
	}

	void try_write_audio_buffer()
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
					spectrum.resize(stereo ? (pill_count() / 2) : pill_count());
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

	int leftmostPillX() const { return margin + (pill.width / 2); }
	int pillY() const { return window.GetHeight() - margin - (pill.width / 2); }

	void render_spectrum_full()
	{
		for (int i = 0; i < (int)spectrum.size(); ++i)
		{
			const auto [r, g, b] = apply_wheel_coloring(i, [this](const int i)
														{ return (float)i / spectrum.size(); });
			renderer.drawPillFromBottomRGBA(
				leftmostPillX() + (i * (pill.width + pill.padding)), pillY(), pill.width,
				multiplier * abs(spectrum[i]) * window.GetHeight(), r, g, b, 255);
		}
	}

	void print_half(int half)
	{
		const auto half_width = pill_count() / 2;
		switch (half)
		{
		case 1:
			for (int i = half_width; i >= 0; --i)
			{
				const auto [r, g, b] = apply_wheel_coloring(i, [&](const int i)
															{ return (float)(half_width - i) / half_width; });
				drawPillAt(i, half_width - i, r, g, b, 255);
			}
			break;

		case 2:
			for (int i = half_width + 1; i < pill_count(); ++i)
			{
				const auto [r, g, b] = apply_wheel_coloring(i, [&](const int i)
															{ return (float)i / half_width; });
				drawPillAt(i, i - half_width, r, g, b, 255);
			}
			break;

		default:
			throw std::logic_error("Visualizer::print_half: half can only be 1 or 2");
		}
	}

	void drawPillAt(const int pill_index, const int spectrum_index, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a)
	{
		renderer.drawPillFromBottomRGBA(
			leftmostPillX() + (pill_index * (pill.width + pill.padding)), pillY(), pill.width,
			multiplier * abs(spectrum[spectrum_index]) * window.GetHeight(), r, g, b, a);
	}

	std::tuple<Uint8, Uint8, Uint8> apply_wheel_coloring(const int i, const std::function<float(int)> &ratio_calc)
	{
		const auto [h, s, v] = wheel.hsv;
		return ColorUtils::hsvToRgb(ratio_calc(i) + h, s, v);
	}
};
