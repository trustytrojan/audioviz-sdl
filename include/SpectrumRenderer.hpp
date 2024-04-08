#pragma once

#include "MyRenderer.hpp"
#include "FrequencySpectrum.hpp"
#include "ColorUtils.hpp"

class SpectrumRenderer : public MyRenderer
{
public:
	using RGBTuple = std::tuple<Uint8, Uint8, Uint8>;

	enum class ColorMode
	{
		WHEEL,
		SOLID
	};

	enum class BarType
	{
		RECTANGLE,
		PILL
	};

protected:
	using FS = FrequencySpectrum;

	float multiplier = 4;
	FS fs;

	/**
	 * Output vector for storing spectrum data from calls to `FrequencySpectrum::render`.
	 * As mentioned in `FrequencySpectrum`, the size of this vector is used to determine
	 * the number of frequency bins to map the FFT output to. Subclasses are expected
	 * to set this vector's size appropriately for visual rendering purposes.
	 */
	std::vector<float> spectrum;

public:
	SpectrumRenderer(const int sample_size, SDL2pp::Window &window, const Uint32 flags)
		: MyRenderer(window, flags),
		  fs(sample_size) {}

	// color stuff
	class
	{
		friend class SpectrumRenderer;
		ColorMode mode = ColorMode::WHEEL;
		RGBTuple solid_rgb{255, 255, 255};

	public:
		void set_mode(const ColorMode mode) { this->mode = mode; }
		void set_solid_rgb(const RGBTuple &rgb) { solid_rgb = rgb; }

		/**
		 * @param index_ratio the ratio of your loop index (aka `i`) to the total number of bars to print (aka `spectrum.size()`)
		 */
		RGBTuple get(const float index_ratio) const
		{
			switch (mode)
			{
			case ColorMode::WHEEL:
			{
				const auto [h, s, v] = wheel.hsv;
				return ColorUtils::hsvToRgb(index_ratio + h + wheel.time, s, v);
			}

			case ColorMode::SOLID:
				return solid_rgb;

			default:
				throw std::logic_error("SpectrumRenderer::color::get: default case hit");
			}
		}

		class
		{
			friend class SpectrumRenderer;
			float time = 0, rate = 0;
			// hue offset, saturation, value
			std::tuple<float, float, float> hsv{0.9, 0.7, 1};

		public:
			void set_rate(const float rate) { this->rate = rate; }
			void set_hsv(const std::tuple<float, float, float> &hsv) { this->hsv = hsv; }
			void increment() { time += rate; }
		} wheel;
	} color;

	// bar stuff
	class
	{
		friend class SpectrumRenderer;
		int width = 10, spacing = 5;
		BarType type = BarType::PILL;

	public:
		void set_width(const int width) { this->width = width; }
		void set_spacing(const int spacing) { this->spacing = spacing; }
		void set_type(const BarType type) { this->type = type; }
	} bar;

	void set_sample_size(const int sample_size);
	void set_multiplier(const float multiplier);
	void set_interp_type(const FS::InterpolationType interp_type);
	void set_scale(const FS::Scale scale);
	void set_nth_root(const int nth_root);
	void set_accum_method(const FS::AccumulationMethod method);
	void set_window_func(const FS::WindowFunction wf);
	void copy_channel_to_input(const float *audio, int num_channels, int channel, bool interleaved);

	// Assumes you have already called `copy_channel_to_input` beforehand.
	void render_spectrum(const SDL2pp::Rect &rect, const bool backwards);
};