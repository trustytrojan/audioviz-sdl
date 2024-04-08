#pragma once

#include <sndfile.hh>
#include <chrono>
#include "PortAudio.hpp"
#include "SpectrumRenderer.hpp"

class Visualizer
{
public:
	using FS = FrequencySpectrum;
	using SR = SpectrumRenderer;

protected:
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

	int mono = -1;

public:
	// width and height matter if you are pre-rendering!
	Visualizer(const std::string &audio_file, int width = 800, int height = 600);

	void start();
	void encode_to_video_popen(const std::string &output_file);
	void do_actual_rendering();

	void set_width(int width);
	void set_height(int height);
	void set_mono(int mono);

	/**
	 * Set the sample chunk size to use in internal calculations.
	 * @note Smaller values increase responsiveness, but decrease accuracy. Larger values do the opposite.
	 * @note This method is thread safe.
	 * @param sample_size new sample size to use
	 * @return reference to self
	 */
	void set_sample_size(int sample_size);

	/**
	 * Set the multiplier to multiply the spectrum's height by.
	 * @param multiplier new multiplier to use
	 * @return reference to self
	 */
	void set_multiplier(float multiplier);

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	void set_interp_type(FS::InterpolationType interp_type);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	void set_scale(FS::Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	void set_nth_root(int nth_root);

	/**
	 * Set frequency bin accumulation method.
	 * @note Choosing `SUM` results in more visible detail in the treble frequencies, at the cost of their amplitudes being visually exaggerated.
	 * @note Choosing `MAX` results in a more true-to-waveform frequency distribution, however treble frequencies aren't very visible.
	 * @param interp new accumulation method to use
	 * @returns reference to self
	 */
	void set_accum_method(FS::AccumulationMethod method);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	void set_window_function(FS::WindowFunction wf);

	void set_bar_type(SR::BarType type);
	void set_bar_width(int width);
	void set_bar_spacing(int spacing);

	void set_color_mode(SR::ColorMode mode);
	void set_color_wheel_rate(float rate);
	void set_color_solid_rgb(const SR::RGBTuple &rgb);
	void set_color_wheel_hsv(const std::tuple<float, float, float> &hsv);

private:
	int audio_frames_per_video_frame() const { return sf.samplerate() / refresh_rate; }
	void print_render_stats(int frame, const std::chrono::_V2::system_clock::time_point &start);
	void try_write_audio_buffer(PortAudio::Stream &pa_stream);
	void handle_events();
};
