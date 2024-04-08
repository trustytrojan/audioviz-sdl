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

Visualizer &Visualizer::set_interp_type(const FS::InterpolationType interp_type)
{
	sr.set_interp_type(interp_type);
	return *this;
}

Visualizer &Visualizer::set_scale(const FS::Scale scale)
{
	sr.set_scale(scale);
	return *this;
}

Visualizer &Visualizer::set_nth_root(const int nth_root)
{
	sr.set_nth_root(nth_root);
	return *this;
}

Visualizer &Visualizer::set_accum_method(const FS::AccumulationMethod method)
{
	sr.set_accum_method(method);
	return *this;
}

Visualizer &Visualizer::set_window_function(const FS::WindowFunction wf)
{
	sr.set_window_func(wf);
	return *this;
}

Visualizer &Visualizer::set_bar_type(const SR::BarType type)
{
	sr.bar.set_type(type);
	return *this;
}

Visualizer &Visualizer::set_bar_width(const int width)
{
	sr.bar.set_width(width);
	return *this;
}

Visualizer &Visualizer::set_bar_spacing(const int spacing)
{
	sr.bar.set_spacing(spacing);
	return *this;
}
