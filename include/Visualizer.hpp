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
	inline static const char *const blurred_image = ".blurred.jpg";

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

	// if nonnegative, forces a mono spectrum with the specified channel
	int mono = -1;

	// background image filepath
	SDL2pp::Optional<SDL2pp::Texture> bg_texture_opt;

public:
	// width and height matter if you are pre-rendering!
	Visualizer(const std::string &audio_file, int width = 800, int height = 600);

	/**
	 * Starts rendering the visualizer to the window.
	 * Plays the audio while rendering.
	 */
	void start();

	/**
	 * Uses `ffmpeg` to encode rendered frames to a video.
	 * @param output_file output video filename; `ffmpeg` is invoked with `-y` so the file will be overwritten
	 * @param fps desired frame rate of the video.
	 * @param vcodec desired video codec. by default does not pass a `-c:v` argument to `ffmpeg`. can error if the codec is incompatible with the output container.
	 * @param acodec desired audio codec. by default passes `-c:a copy` to `ffmpeg` which can error if the audio's codec is incompatible with the output container.
	 */
	void encode_to_video(const std::string &output_file, int fps, const std::string &vcodec = "h264", const std::string &acodec = "copy");

	void set_width(int width);
	void set_height(int height);
	void set_mono(int mono);

	/**
	 * Set a background image for the spectrum.
	 * @param filepath path to image file, or empty string to disable background
	 */
	void set_background(const std::string &filepath);

	/**
	 * Set the sample chunk size to use in internal calculations.
	 * @note Smaller values increase responsiveness, but decrease accuracy. Larger values do the opposite.
	 * @note This method is thread safe.
	 * @param sample_size new sample size to use
	 */
	void set_sample_size(int sample_size);

	/**
	 * Set the multiplier to multiply the spectrum's height by.
	 * @param multiplier new multiplier to use
	 */
	void set_multiplier(float multiplier);

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 */
	void set_interp_type(FS::InterpolationType interp_type);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 */
	void set_scale(FS::Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	void set_nth_root(int nth_root);

	/**
	 * Set frequency bin accumulation method.
	 * @note Choosing `SUM` results in more visible detail in the treble frequencies, at the cost of their amplitudes being visually exaggerated.
	 * @note Choosing `MAX` results in a more true-to-waveform frequency distribution, however treble frequencies aren't very visible.
	 * @param interp new accumulation method to use
	 */
	void set_accum_method(FS::AccumulationMethod method);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 */
	void set_window_function(FS::WindowFunction wf);

	/**
	 * Set bar type.
	 * @param type new bar type
	 */
	void set_bar_type(SR::BarType type);

	/**
	 * Set bar width.
	 * @param width new bar width
	 */
	void set_bar_width(int width);

	/**
	 * Set bar spacing.
	 * @param spacing new bar spacing
	 */
	void set_bar_spacing(int spacing);

	/**
	 * Set color mode.
	 * @param mode new color mode
	 */
	void set_color_mode(SR::ColorMode mode);

	/**
	 * Set the rate at which the color wheel's hue offset shifts per frame.
	 * @param rate new color wheel hue offset change rate
	 */
	void set_color_wheel_rate(float rate);

	/**
	 * Set the solid color to use when the color mode is `SOLID`.
	 * @param rgb new solid color
	 */
	void set_color_solid_rgb(const SR::RGBTuple &rgb);

	/**
	 * Set the (hue offset, saturation, value) tuple used in `SpectrumRenderer` for coloring spectrum bars.
	 * @param hsv new (hue offset, saturation, value) tuple
	 */
	void set_color_wheel_hsv(const std::tuple<float, float, float> &hsv);

private:
	void handle_events();
	void do_actual_rendering();
	SDL2pp::Rect bg_texture_centered_max_width();
};
