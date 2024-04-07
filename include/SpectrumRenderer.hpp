#pragma once

#include "MyRenderer.hpp"
#include "FrequencySpectrum.hpp"

class SpectrumRenderer : protected MyRenderer
{
public:
	using FS = FrequencySpectrum;

	enum class ColorMode
	{
		NONE,
		WHEEL,
		SOLID
	};

protected:
	int sample_size = 3000;
	float multiplier = 5;
	FS fs = sample_size;

	/**
	 * Output vector for storing spectrum data from calls to `FrequencySpectrum::render`.
	 * As mentioned in `FrequencySpectrum`, the size of this vector is used to determine
	 * the number of frequency bins to map the FFT output to. Subclasses are expected
	 * to set this vector's size appropriately for visual rendering purposes.
	 */
	std::vector<float> spectrum;

public:
	void render_spectrum(const float *const audio, const int num_channels, const int channel, const bool interleaved)
	{
		fs.copy_channel_to_input(audio, num_channels, channel, interleaved);
		fs.render(spectrum);
		do_actual_rendering();
	}

	void set_sample_size(const int sample_size)
	{
		this->sample_size = sample_size;
		fs.set_fft_size(sample_size);
	}

	void set_interp_type(const FS::InterpolationType interp_type)
	{
		fs.set_interp_type(interp_type);
	}

	void set_scale(const FS::Scale scale)
	{
		fs.set_scale(scale);
	}

	void set_nth_root(const int nth_root)
	{
		fs.set_nth_root(nth_root);
	}

	void set_accum_method(const FS::AccumulationMethod method)
	{
		fs.set_accum_method(method);
	}

	void set_window_function(const FS::WindowFunction wf)
	{
		fs.set_window_func(wf);
	}

protected:
	virtual void do_actual_rendering() = 0;
};