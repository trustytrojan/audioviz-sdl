#include "FrequencySpectrum.hpp"
#include <stdexcept>

FrequencySpectrum &FrequencySpectrum::set_fft_size(const int fft_size)
{
	fftw.set_n(fft_size);
	fftsize_inv = 1. / fft_size;
	scale_max.set(*this);
	return *this;
}

FrequencySpectrum &FrequencySpectrum::set_interp_type(const InterpType interp)
{
	this->interp = interp;
	return *this;
}

FrequencySpectrum &FrequencySpectrum::set_window_func(const WindowFunction wf)
{
	this->wf = wf;
	return *this;
}

FrequencySpectrum &FrequencySpectrum::set_accum_method(const AccumulationMethod am)
{
	this->am = am;
	return *this;
}

FrequencySpectrum &FrequencySpectrum::set_scale(const Scale scale)
{
	this->scale = scale;
	return *this;
}

FrequencySpectrum &FrequencySpectrum::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument("FrequencySpectrun::set_nth_root: nth_root cannot be zero!");
	this->nth_root = nth_root;
	nth_root_inverse = 1.f / nth_root;
	return *this;
}