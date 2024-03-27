#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#include <cstdlib>
#include <array>

#include <sndfile.h>
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <portaudio.h>
#include <argparse/argparse.hpp>

using argparse::ArgumentParser;

struct TerminalSize
{
	int width, height;

	TerminalSize()
	{
		winsize ws;

		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
		{
			perror("ioctl");
			throw std::runtime_error("ioctl() failed");
		}

		width = ws.ws_col;
		height = ws.ws_row;
	}

	bool operator!=(const TerminalSize &other)
	{
		return width != other.width || height != other.height;
	}
};

struct Args : private ArgumentParser
{
	enum class Scale
	{
		LINEAR,
		LOG,
		SQRT
	};

	std::string audio_file;
	int fft_size, half_size;
	char spectrum_char;
	Scale scale;

	Args(const int argc, const char *const *const argv)
		: ArgumentParser(argv[0])
	{
		add_argument("audio_file")
			.help("audio file to visualize and play");
		add_argument("-n", "--sample-size")
			.help("number of samples (or frames of samples) to process at a time; higher -> increases accuracy, lower -> increases responsiveness")
			.default_value(1024)
			.scan<'i', int>()
			.validate();
		add_argument("-c", "--spectrum-char")
			.help("character to render the spectrum with")
			.default_value("#");
		add_argument("--scale")
			.help("spectrum frequency scale")
			.choices("linear", "log", "sqrt")
			.nargs(1)
			.default_value("linear")
			.validate();

		try
		{
			parse_args(argc, argv);
		}
		catch (const std::exception &e)
		{
			// print help to stderr
			std::cerr << *this;

			// rethrow to main
			throw std::runtime_error(e.what());
		}

		audio_file = get("audio_file");
		if ((fft_size = get<int>("-n")) & 1)
			throw std::runtime_error("sample size must be even!");
		half_size = fft_size / 2;
		spectrum_char = get("-c").front();

		const auto &scale = get("--scale");
		if (scale == "linear")
			this->scale = Scale::LINEAR;
		else if (scale == "log")
			this->scale = Scale::LOG;
		else if (scale == "sqrt")
			this->scale = Scale::SQRT;
		else
			throw std::runtime_error("impossible!!!!");
	}
};

void _main(const Args &args)
{
	// open audio file
	SF_INFO sfinfo;
	const auto file = sf_open(args.audio_file.c_str(), SFM_READ, &sfinfo);
	if (!file)
		throw std::runtime_error("failed to open audio file: " + args.audio_file);

	// kissfft initialization
	const auto cfg = kiss_fftr_alloc(args.fft_size, 0, NULL, NULL);

	// PortAudio initialization
	PaError err;

	if ((err = Pa_Initialize()))
		throw std::runtime_error(Pa_GetErrorText(err));

	PaStream *stream;

	if ((err = Pa_OpenDefaultStream(&stream, 0, sfinfo.channels, paFloat32, sfinfo.samplerate, args.fft_size, NULL, NULL)))
		throw std::runtime_error(Pa_GetErrorText(err));

	if ((err = Pa_StartStream(stream)))
		throw std::runtime_error(Pa_GetErrorText(err));

	// arrays to store fft and audio data
	float timedata[args.fft_size];
	kiss_fft_cpx freqdata[args.half_size + 1];
	float buffer[args.fft_size * sfinfo.channels];

	// get terminal size, initialize amplitudes array
	TerminalSize tsize;
	std::vector<float> amplitudes(tsize.width);

	const auto logmax = log(args.half_size + 1);

	// start reading, playing, and processing audio
	while (1)
	{
		{ // if terminal size changes, resize amplitudes array if width changes
			const TerminalSize new_tsize;
			if (tsize != new_tsize)
			{
				if (tsize.width != new_tsize.width)
					amplitudes.resize(new_tsize.width);
				tsize = new_tsize;
			}
		}

		const auto [width, height] = tsize;

		{ // read audio into buffer, write to output stream to play
			// sf_readf_float reads FRAMES, where each FRAME is a COLLECTION of samples, one FOR EACH CHANNEL.
			const auto frames_read = sf_readf_float(file, buffer, args.fft_size);

			// break on end of file
			if (!frames_read)
				break;

			// play the audio as it is read
			Pa_WriteStream(stream, buffer, frames_read);
		}

		// only consider the last channel
		for (int i = 0; i < args.fft_size; ++i)
			timedata[i] = buffer[i * sfinfo.channels];

		// perform fft: frequency range to amplitude values are stored in freqdata
		kiss_fftr(cfg, timedata, freqdata);

		// zero out array since we are creating sums
		for (auto &a : amplitudes)
			a = 0;

		// group up freqdata into bigger ranges by summing up amplitudes for the same calculated index
		for (auto i = 0; i <= args.half_size; ++i)
		{
			const auto [re, im] = freqdata[i];
			const float amplitude = sqrt((re * re) + (im * im));
			int index;
			switch (args.scale)
			{
			case Args::Scale::LINEAR:
				index = ((float)i / (args.half_size + 1)) * width;
				break;
			case Args::Scale::LOG:
				index = (log(i ? i : 1) / logmax) * width;
				break;
			case Args::Scale::SQRT:
				index = sqrt(i) / sqrt(args.half_size + 1) * width;
				break;
			default:
				throw std::runtime_error("impossible!!!!!!!");
			}
			amplitudes[index] += amplitude;
		}

		// clear the terminal
		std::cout << "\ec";

		// print the spectrum
		for (int i = 0; i < width; ++i)
		{
			// calculate height based on amplitude
			int bar_height = 0.001 * amplitudes[i] * height;

			// move cursor to (height, i)
			// remember that (0, 0) in a terminal is the top-left corner, so positive y moves the cursor down.
			std::cout << "\e[" << height << ';' << i << 'f';

			// draw the bar upwards `bar_height` high
			for (int j = 0; j < bar_height; ++j)
				// print character, move cursor up 1, move cursor left 1
				std::cout << args.spectrum_char << "\e[1A\e[1D";
		}
	}

	// resource cleanup
	if ((err = Pa_StopStream(stream)))
		throw std::runtime_error(Pa_GetErrorText(err));

	if ((err = Pa_CloseStream(stream)))
		throw std::runtime_error(Pa_GetErrorText(err));

	if ((err = Pa_Terminate()))
		throw std::runtime_error(Pa_GetErrorText(err));

	kiss_fftr_free(cfg);
	sf_close(file);
}

int main(const int argc, const char *const *const argv)
{
	try
	{
		_main(Args(argc, argv));
	}
	catch (const std::exception &e)
	{
		std::cerr << argv[0] << ": " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
