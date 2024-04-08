#pragma once

#include <sndfile.hh>
#include <chrono>
#include "FrequencySpectrum.hpp"
#include "PortAudio.hpp"
#include "SpectrumRenderer.hpp"

class Visualizer
{
public:
	enum class ColorType
	{
		NONE,
		WHEEL,
		SOLID
	};

	using FS = FrequencySpectrum;
	using SR = SpectrumRenderer;

private:
	// general parameters
	int sample_size = 3000;
	const std::string audio_file;

	// SDL2pp window and renderer
	SDL2pp::Window window;
	SR sr = SR(sample_size, window, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// open audio file
	SndfileHandle sf = audio_file;

	// intermediate arrays
	std::vector<float> audio_buffer = std::vector<float>(sample_size * sf.channels());

	// synchronization
	int refresh_rate = [this]
	{
		SDL_DisplayMode mode;
		window.GetDisplayMode(mode);
		return mode.refresh_rate;
	}();

	// total spectrum/video frames to render from audio
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

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	Visualizer &set_interp_type(const FS::InterpolationType interp_type);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	Visualizer &set_scale(const FS::Scale scale);

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
	Visualizer &set_accum_method(const FS::AccumulationMethod method);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	Visualizer &set_window_function(const FS::WindowFunction wf);

	Visualizer &set_bar_type(const SR::BarType type);
	Visualizer &set_bar_width(const int width);
	Visualizer &set_bar_spacing(const int spacing);

private:
	int audio_frames_per_video_frame() const { return sf.samplerate() / refresh_rate; }
	void print_render_stats(const int frame, const std::chrono::_V2::system_clock::time_point &start);
	void try_write_audio_buffer(PortAudio::Stream &pa_stream);
	void handleEvents();
};
