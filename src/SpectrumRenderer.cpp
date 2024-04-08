#include "SpectrumRenderer.hpp"

void SpectrumRenderer::render_spectrum(const SDL2pp::Rect &rect, const bool backwards)
{
	// resize spectrum first! this is the old formula, except now it's relative to the passed in rect.
	// this makes things much much more flexible
	spectrum.resize((rect.GetW() - bar.spacing) / (bar.width + bar.spacing));

	// render spectrum
	fs.render(spectrum);

	for (int i = 0; i < (int)spectrum.size(); ++i)
	{
		const auto [r, g, b] = color.get((float)i / spectrum.size());
		const auto x = backwards ? (rect.GetBottomRight().GetX() - bar.width - (i * (bar.width + bar.spacing)))
								 : (rect.GetX() + (i * (bar.width + bar.spacing)));
		const auto h = std::min((float)rect.GetH(),
								multiplier * std::max(0.f, spectrum[i]) * rect.GetH());
		drawBarFromBottom(x, rect.GetH(), bar.width, h, r, g, b, 255);
	}

	color.wheel.increment();
}
