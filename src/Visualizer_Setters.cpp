#include "Visualizer.hpp"

void Visualizer::set_background(const std::string &filepath)
{
	if (filepath.size())
		bg_texture_opt.emplace(sr, filepath);
	else
		bg_texture_opt.reset();
}

void Visualizer::set_ffmpeg_path(const std::string &path)
{
	ffmpeg_path = path;
}

void Visualizer::set_width(const int width)
{
	window.SetSize(width, window.GetHeight());
}

void Visualizer::set_height(const int height)
{
	window.SetSize(window.GetWidth(), height);
}

void Visualizer::set_mono(const int mono)
{
	this->mono = mono;
}

void Visualizer::set_sample_size(const int sample_size)
{
	this->sample_size = sample_size;
	sr.set_sample_size(sample_size);
	audio_buffer.resize(sample_size * sf.channels());
}

void Visualizer::set_multiplier(const float multiplier)
{
	sr.set_multiplier(multiplier);
}

void Visualizer::set_interp_type(const FS::InterpolationType interp_type)
{
	sr.set_interp_type(interp_type);
}

void Visualizer::set_scale(const FS::Scale scale)
{
	sr.set_scale(scale);
}

void Visualizer::set_nth_root(const int nth_root)
{
	sr.set_nth_root(nth_root);
}

void Visualizer::set_accum_method(const FS::AccumulationMethod method)
{
	sr.set_accum_method(method);
}

void Visualizer::set_window_function(const FS::WindowFunction wf)
{
	sr.set_window_func(wf);
}

void Visualizer::set_bar_type(const SR::BarType type)
{
	sr.bar.set_type(type);
}

void Visualizer::set_bar_width(const int width)
{
	sr.bar.set_width(width);
}

void Visualizer::set_bar_spacing(const int spacing)
{
	sr.bar.set_spacing(spacing);
}

void Visualizer::set_color_mode(const SR::ColorMode mode)
{
	sr.color.set_mode(mode);
}

void Visualizer::set_color_wheel_rate(const float rate)
{
	sr.color.wheel.set_rate(rate);
}

void Visualizer::set_color_solid_rgb(const SR::RGBTuple &rgb)
{
	sr.color.set_solid_rgb(rgb);
}

void Visualizer::set_color_wheel_hsv(const std::tuple<float, float, float> &hsv)
{
	sr.color.wheel.set_hsv(hsv);
}
