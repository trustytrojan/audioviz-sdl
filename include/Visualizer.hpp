#pragma once

#include <sndfile.hh>
#include <chrono>
#include "MyRenderer.hpp"
#include "FrequencySpectrum.hpp"
#include "PortAudio.hpp"

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
	const std::string &audio_file;

	struct
	{
		int width = 10, padding = 5;
	} pill;

	int margin = 15;
	int pill_count = (window.GetWidth() - (2 * margin)) / (pill.width + pill.padding);

	// SDL2pp window and renderer
	SDL2pp::Window window;
	MyRenderer renderer = MyRenderer(window, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// initialize spectrum renderer
	FrequencySpectrum fs = sample_size;

	// open audio file
	SndfileHandle sf = audio_file;
	bool stereo = (sf.channels() == 2);

	// intermediate arrays
	std::vector<float>
		audio_buffer = std::vector<float>(sample_size * sf.channels()),
		spectrum;

	// synchronization
	int refresh_rate = [this]
	{
		SDL_DisplayMode mode;
		window.GetDisplayMode(mode);
		return mode.refresh_rate;
	}();

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
	// bool prerender = false;
	// std::queue<std::vector<float>> spectrum_queue;

	const int total_frames = sf.frames() / audio_frames_per_video_frame();

public:
	// width and height matter if you are pre-rendering!
	Visualizer(const std::string &audio_file, const int width = 800, const int height = 600);

	void start();
	void encode_to_video_popen(const std::string &output_file);

	Visualizer &set_width(const int width);
	Visualizer &set_height(const int height);

	/**
	 * Set the sample chunk size to use in internal calculations.
	 * @note Smaller values increase responsiveness, but decrease accuracy. Larger values do the opposite.
	 * @note This method is thread safe.
	 * @param sample_size new sample size to use
	 * @return reference to self
	 */
	Visualizer &set_sample_size(const int sample_size);

	/**
	 * Set the multiplier to multiply the spectrum's height by.
	 * @param multiplier new multiplier to use
	 * @return reference to self
	 */
	Visualizer &set_multiplier(const float multiplier);

	// Visualizer &set_prerender(bool prerender);

	Visualizer &set_stereo(bool enabled);

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	Visualizer &set_interp_type(const InterpType interp_type);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	Visualizer &set_scale(const Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	Visualizer &set_nth_root(const int nth_root);

	/**
	 * Set frequency bin accumulation method.
	 * @note Choosing `SUM` results in more visible detail in the treble frequencies, at the cost of their amplitudes being visually exaggerated.
	 * @note Choosing `MAX` results in a more true-to-waveform frequency distribution, however treble frequencies aren't very visible.
	 * @param interp new accumulation method to use
	 * @returns reference to self
	 */
	Visualizer &set_accum_method(const AccumulationMethod method);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	Visualizer &set_window_function(const WindowFunction wf);

	Visualizer &set_pill_width(const int width);

	Visualizer &set_pill_padding(const int padding);

	Visualizer &set_margin(const int margin);

private:
	int leftmostPillX() const { return margin + (pill.width / 2); }
	int pillY() const { return window.GetHeight() - margin - (pill.width / 2); }
	int audio_frames_per_video_frame() const { return sf.samplerate() / refresh_rate; }
	void resize_spectrum_array();

	void start_sync();
	void do_actual_rendering();
	void print_render_stats(const int frame, const std::chrono::nanoseconds draw_time, const double fps);
	void try_write_audio_buffer(PortAudio::Stream &pa_stream);
	void handleEvents();
	void copy_channel_to_input(const int channel_num);
	void print_full();
	void print_half(int half);
	void drawPillAt(const int pill_index, const int spectrum_index, const Uint8 r, const Uint8 g, const Uint8 b, const Uint8 a);
	std::tuple<Uint8, Uint8, Uint8> apply_wheel_coloring(const int i, const std::function<float(int)> &ratio_calc);
};
