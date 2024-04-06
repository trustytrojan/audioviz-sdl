#pragma once

#include <vector>
#include "spline.hpp"
#include "fftwf_dft_r2c_1d.hpp"

class FrequencySpectrum
{
public:
	enum class Scale
	{
		LINEAR,
		LOG,
		NTH_ROOT
	};

	enum class InterpType
	{
		NONE,
		LINEAR = tk::spline::linear,
		CSPLINE = tk::spline::cspline,
		CSPLINE_HERMITE = tk::spline::cspline_hermite
	};

	enum class AccumulationMethod
	{
		SUM,
		MAX
	};

	enum class WindowFunction
	{
		NONE,
		HANNING,
		HAMMING,
		BLACKMAN
	};

private:
	// fft size
	int fft_size;
	float fftsize_inv = 1.f / fft_size;

	// nth root
	int nth_root = 2;
	float nth_root_inverse = 1.f / nth_root;

	// fftw initialization
	fftwf_dft_r2c_1d fftw = fft_size;

	// interpolation
	tk::spline spline;
	InterpType interp = InterpType::CSPLINE;

	// output spectrum scale
	Scale scale = Scale::LOG;

	// method for accumulating amplitudes in frequency bins
	AccumulationMethod am = AccumulationMethod::MAX;

	// window function
	WindowFunction wf = WindowFunction::BLACKMAN;

	// struct to hold the "max"s used in `calc_index_ratio`
	struct
	{
		double linear, log, sqrt, cbrt, nthroot;
		void set(const FrequencySpectrum &fs)
		{
			const auto max = fs.fftw.get_output_size();
			linear = max;
			log = ::log(max);
			sqrt = ::sqrt(max);
			cbrt = ::cbrt(max);
			nthroot = ::pow(max, fs.nth_root_inverse);
		}
	} scale_max;

public:
	/**
	 * Initialize frequency spectrum renderer.
	 * @param fft_size sample chunk size fed into the `transform` method
	 */
	FrequencySpectrum(const int fft_size);

	/**
	 * Set the FFT size used in the `kissfft` library.
	 * @param fft_size new fft size to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `fft_size` is not even
	 */
	FrequencySpectrum &set_fft_size(const int fft_size);

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	FrequencySpectrum &set_interp_type(const InterpType interp);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	FrequencySpectrum &set_window_func(const WindowFunction wf);

	/**
	 * Set frequency bin accumulation method.
	 * @param interp new accumulation method to use
	 * @returns reference to self
	 */
	FrequencySpectrum &set_accum_method(const AccumulationMethod am);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	FrequencySpectrum &set_scale(const Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	FrequencySpectrum &set_nth_root(const int nth_root);

	float *input_array() { return fftw.get_input(); }

	// it is assumed that `input_array()` holds your input wave data!
	// you must write your input data to `input_array()` before calling `render`!!!!!!!!
	void render(std::vector<float> &spectrum);

private:
	void apply_window_func(float *const timedata);
	int calc_index(const int i, const int max_index);
	float calc_index_ratio(const float i);
	void interpolate(std::vector<float> &spectrum);
};