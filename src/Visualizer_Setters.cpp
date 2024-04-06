#include "Visualizer.hpp"

Visualizer &Visualizer::set_width(const int width)
{
	window.SetSize(width, window.GetHeight());
	return *this;
}

Visualizer &Visualizer::set_height(const int height)
{
	window.SetSize(window.GetWidth(), height);
	return *this;
}

Visualizer &Visualizer::set_sample_size(const int sample_size)
{
	this->sample_size = sample_size;
	fs.set_fft_size(sample_size);
	audio_buffer.resize(sample_size * sf.channels());
	return *this;
}

Visualizer &Visualizer::set_multiplier(const float multiplier)
{
	this->multiplier = multiplier;
	return *this;
}

// Visualizer &Visualizer::set_prerender(bool prerender)
// {
// 	this->prerender = prerender;
// 	return *this;
// }

Visualizer &Visualizer::set_stereo(bool enabled)
{
	stereo = enabled;
	return *this;
}

Visualizer &Visualizer::set_interp_type(const InterpType interp_type)
{
	fs.set_interp_type(interp_type);
	return *this;
}

Visualizer &Visualizer::set_scale(const Scale scale)
{
	fs.set_scale(scale);
	return *this;
}

Visualizer &Visualizer::set_nth_root(const int nth_root)
{
	fs.set_nth_root(nth_root);
	return *this;
}

Visualizer &Visualizer::set_accum_method(const AccumulationMethod method)
{
	fs.set_accum_method(method);
	return *this;
}

Visualizer &Visualizer::set_window_function(const WindowFunction wf)
{
	fs.set_window_func(wf);
	return *this;
}

Visualizer &Visualizer::set_pill_width(const int width)
{
	pill.width = width;
	return *this;
}

Visualizer &Visualizer::set_pill_padding(const int padding)
{
	pill.padding = padding;
	return *this;
}

Visualizer &Visualizer::set_margin(const int margin)
{
	this->margin = margin;
	return *this;
}
