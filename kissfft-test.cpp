#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <memory>

#include <sndfile.h>
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <portaudio.h>

#define FFT_SIZE 1024

void get_terminal_size(int &width, int &height)
{
	struct winsize size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
	width = size.ws_col;
	height = size.ws_row;
}

void print_spectrum(float *amplitudes, int bin_count, int height)
{
	for (int i = 0; i < bin_count; ++i)
	{
		int bar_height = amplitudes[i] * height;
		std::cout << "\e[" << height << ';' << i << 'f';
		// int max = std::min(bar_height, height);
		for (int j = 0; j < bar_height; ++j)
		{
			std::cout << "#\e[1A\e[1D";
		}
	}
}

void clear_terminal()
{
	std::cout << "\033c";
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 2)
	{
		std::cerr << "audio file required\n";
		return EXIT_FAILURE;
	}

	SF_INFO sfinfo;
	SNDFILE *const file = sf_open(argv[1], SFM_READ, &sfinfo);

	if (!file)
	{
		std::cerr << "Could not open file\n";
		return EXIT_FAILURE;
	}

	const auto cfg = kiss_fftr_alloc(FFT_SIZE, 0, NULL, NULL);
	const auto timedata = new kiss_fft_scalar[FFT_SIZE];
	const auto freqdata = new kiss_fft_cpx[FFT_SIZE];
	const auto buffer = new float[FFT_SIZE * sfinfo.channels];

	sf_count_t frame_count = 0;

	PaError err;

	if (err = Pa_Initialize())
	{
		std::cerr << Pa_GetErrorText(err) << '\n';
		return EXIT_FAILURE;
	}

	PaStream *stream;

	err = Pa_OpenDefaultStream(
		&stream,
		0,				 // no input channels
		sfinfo.channels, // stereo output
		paFloat32,		 // 32 bit floating point output
		sfinfo.samplerate,
		FFT_SIZE, // frames per buffer
		NULL,	  // callback is not used
		NULL);	  // no callback data

	if (err)
	{
		std::cerr << Pa_GetErrorText(err) << '\n';
		return EXIT_FAILURE;
	}

	if (err = Pa_StartStream(stream))
	{
		std::cerr << Pa_GetErrorText(err) << '\n';
		return EXIT_FAILURE;
	}

	while (1)
	{
		// sf_readf_float reads FRAMES into buffer, where each FRAME is a COLLECTION of samples, one FOR EACH CHANNEL.
		const auto frames = sf_readf_float(file, buffer, FFT_SIZE);

		if ((err = Pa_WriteStream(stream, buffer, frames)) != paNoError)
		{
			// don't bother, it's always an output underflow,
			// and the music sounds great regardless.

			// std::cerr << Pa_GetErrorText(err) << '\n';
			// return EXIT_FAILURE;
		}

		if (!frames)
			break;

		// Only consider the last channel
		for (int i = 0; i < FFT_SIZE; ++i)
			timedata[i] = buffer[i * sfinfo.channels];

		kiss_fftr(cfg, timedata, freqdata);

		// Get terminal width and height
		int width, height;
		get_terminal_size(width, height);

		// Group up frequencies into larger bins, to accomodate for terminal width
		int bin_count = width;
		float amplitudes[bin_count];

		for (int i = 0; i < bin_count; ++i)
			amplitudes[i] = 0;

		for (int i = 0; i < (FFT_SIZE / 2); ++i)
		{
			float amplitude = 0.01 * sqrt(freqdata[i].r * freqdata[i].r + freqdata[i].i * freqdata[i].i);
			int bin_index = round(((float)i / (float)(FFT_SIZE / 2)) * (float)bin_count);
			amplitudes[bin_index] += amplitude;
		}

		clear_terminal();
		print_spectrum(amplitudes, bin_count, height);

		frame_count += frames;
	}

	if ((err = Pa_StopStream(stream)) != paNoError)
	{
		std::cerr << Pa_GetErrorText(err) << '\n';
		return EXIT_FAILURE;
	}

	if ((err = Pa_CloseStream(stream)) != paNoError)
	{
		std::cerr << Pa_GetErrorText(err) << '\n';
		return EXIT_FAILURE;
	}

	Pa_Terminate();

	delete[] timedata;
	delete[] freqdata;
	delete[] buffer;
	sf_close(file);
}