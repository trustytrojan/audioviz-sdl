#include "SpectrumRenderer.hpp"

void SpectrumRenderer::set_sample_size(const int sample_size)
{
	fs.set_fft_size(sample_size);
}

void SpectrumRenderer::set_multiplier(const float multiplier)
{
	this->multiplier = multiplier;
}

void SpectrumRenderer::set_interp_type(const FS::InterpolationType interp_type)
{
	fs.set_interp_type(interp_type);
}

void SpectrumRenderer::set_scale(const FS::Scale scale)
{
	fs.set_scale(scale);
}

void SpectrumRenderer::set_nth_root(const int nth_root)
{
	fs.set_nth_root(nth_root);
}

void SpectrumRenderer::set_accum_method(const FS::AccumulationMethod method)
{
	fs.set_accum_method(method);
}

void SpectrumRenderer::set_window_func(const FS::WindowFunction wf)
{
	fs.set_window_func(wf);
}