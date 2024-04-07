#pragma once

#include "SpectrumRenderer.hpp"

class MonoSpectrumRenderer : protected SpectrumRenderer
{
protected:
	void render_spectrum(const float *const audio)
	{
		SpectrumRenderer::render_spectrum(audio, 1, 1, false);
	}

	virtual void do_actual_rendering() override
	{
		for (size_t i = 0; i < spectrum.size(); ++i)
			render_something_with(i);
	}

	virtual void render_something_with(int i) = 0;
};

class BarMonoSpectrumRenderer : MonoSpectrumRenderer
{
	struct
	{
		int width = 10, padding = 5;
	} bar;

	int margin = 15;
	int pill_count = (GetOutputWidth() - (2 * margin)) / (bar.width + bar.padding);

	int leftmostBarX() const { return margin + (bar.width / 2); }
	int barY() const { return GetOutputHeight() - margin - (bar.width / 2); }

protected:
	virtual void render_something_with(int i) override
	{
		const auto [r, g, b] = get_color();
		MyRenderer::drawPillFromBottom(
			leftmostBarX() + (i * (bar.width + bar.padding)), barY(), bar.width,
			multiplier * abs(spectrum[i]) * GetOutputHeight(), r, g, b, 255);
	}
};