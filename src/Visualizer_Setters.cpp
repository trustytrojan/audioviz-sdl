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
	sr.set_sample_size(sample_size);
	audio_buffer.resize(sample_size * sf.channels());
	return *this;
}

Visualizer &Visualizer::set_multiplier(const float multiplier)
{
	sr.set_multiplier(multiplier);
	return *this;
}

Visualizer &Visualizer::set_interp_type(const InterpType interp_type)
{
	sr.set_interp_type(interp_type);
	return *this;
}

Visualizer &Visualizer::set_scale(const Scale scale)
{
	sr.set_scale(scale);
	return *this;
}

Visualizer &Visualizer::set_nth_root(const int nth_root)
{
	sr.set_nth_root(nth_root);
	return *this;
}

Visualizer &Visualizer::set_accum_method(const AccumulationMethod method)
{
	sr.set_accum_method(method);
	return *this;
}

Visualizer &Visualizer::set_window_function(const WindowFunction wf)
{
	sr.set_window_func(wf);
	return *this;
}
