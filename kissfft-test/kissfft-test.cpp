#include <sys/ioctl.h>
#include <unistd.h>

#include <spline.hpp>
#include <kiss_fft.h>
#include <kiss_fftr.h>
#include <portaudio.h>
#include <argparse/argparse.hpp>
#include <sndfile.hh>

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
	int fft_size;
	float multiplier;
	char spectrum_char;
	Scale scale;

	Args(const int argc, const char *const *const argv)
		: ArgumentParser(argv[0])
	{
		add_argument("audio_file")
			.help("audio file to visualize and play");
		add_argument("-n", "--sample-size")
			.help("number of samples (or frames of samples) to process at a time\n- higher -> increases accuracy\n- lower -> increases responsiveness")
			.default_value(2048)
			.scan<'i', int>()
			.validate();
		add_argument("-c", "--spectrum-char")
			.help("character to render the spectrum with")
			.default_value("#");
		add_argument("-s", "--scale")
			.help("spectrum frequency scale")
			.choices("linear", "log", "sqrt")
			.default_value("log")
			.validate();
		add_argument("-m", "--multiplier")
			.help("multiply spectrum amplitude by this amount")
			.default_value(3.f)
			.scan<'f', float>()
			.validate();
		try
		{
			parse_args(argc, argv);
		}
		catch (const std::exception &e)
		{
			// print error and help to stderr
			std::cerr << argv[0] << ": " << e.what() << '\n'
					  << *this;

			// just exit here since we don't want to print anything after the help
			exit(EXIT_FAILURE);
		}

		audio_file = get("audio_file");
		if ((fft_size = get<int>("-n")) & 1)
			throw std::runtime_error("sample size must be even!");
		spectrum_char = get("-c").front();
		multiplier = get<float>("-m");

		const auto &scale = get("-s");
		if (scale == "linear")
			this->scale = Scale::LINEAR;
		else if (scale == "log")
			this->scale = Scale::LOG;
		else if (scale == "sqrt")
			this->scale = Scale::SQRT;
		else
			throw std::runtime_error("impossible!!!!");
	}

	std::tuple<const std::string &, int, float, char, Scale> tuple() const
	{
		return {audio_file, fft_size, multiplier, spectrum_char, scale};
	}
};

struct SmoothedAmplitudes : std::vector<float>
{
	SmoothedAmplitudes(std::vector<float> &amplitudes)
		: std::vector<float>(amplitudes.size())
	{
		// Separate the nonzero values (y's) and their indices (x's)
		std::vector<double> nonzero_values, indices;

		for (int i = 0; i < (int)size(); ++i)
		{
			if (!amplitudes[i])
				continue;
			nonzero_values.push_back(amplitudes[i]);
			indices.push_back(i);
		}

		if (indices.size() >= 3)
		{
			// Create a cubic spline interpolation based on the nonzero values
			tk::spline s(indices, nonzero_values);

			// Use the cubic spline to estimate a value for each position in the list
			for (int i = 0; i < (int)size(); ++i)
				at(i) = amplitudes[i] ? amplitudes[i] : s(i);
		}
		else
			this->swap(amplitudes);
	}
};

class PortAudio
{
	struct Error : std::runtime_error
	{
		Error(const std::string &s) : std::runtime_error("portaudio: " + s) {}
	};

	class Stream
	{
		PaStream *stream;

	public:
		Stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback, void *userData)
		{
			PaError err;
			if ((err = Pa_OpenDefaultStream(&stream, numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData)))
				throw Error(Pa_GetErrorText(err));
			if ((err = Pa_StartStream(stream)))
				throw Error(Pa_GetErrorText(err));
		}

		~Stream()
		{
			PaError err;
			if ((err = Pa_StopStream(stream)))
				std::cerr << Pa_GetErrorText(err) << '\n';
			if ((err = Pa_CloseStream(stream)))
				std::cerr << Pa_GetErrorText(err) << '\n';
		}

		PaError write(const void *const buffer, const size_t n_frames) const
		{
			return Pa_WriteStream(stream, buffer, n_frames);
		}
	};

public:
	PortAudio()
	{
		PaError err;
		if ((err = Pa_Initialize()))
			throw Error(Pa_GetErrorText(err));
	}

	~PortAudio()
	{
		PaError err;
		if ((err = Pa_Terminate()))
			std::cerr << Pa_GetErrorText(err) << '\n';
	}

	Stream stream(int numInputChannels, int numOutputChannels, PaSampleFormat sampleFormat, double sampleRate, unsigned long framesPerBuffer, PaStreamCallback *streamCallback = NULL, void *userData = NULL)
	{
		return Stream(numInputChannels, numOutputChannels, sampleFormat, sampleRate, framesPerBuffer, streamCallback, userData);
	}
};

void _main(const Args &args)
{
	// destructure args
	const auto [audio_file, fft_size, multiplier, spectrum_char, scale] = args.tuple();

	// open audio file
	SndfileHandle sf(audio_file);

	// kissfft initialization
	const auto cfg = kiss_fftr_alloc(fft_size, 0, NULL, NULL);

	// initialize PortAudio, create stream
	PortAudio pa;
	const auto pa_stream = pa.stream(0, sf.channels(), paFloat32, sf.samplerate(), fft_size, NULL, NULL);

	const int freqdata_len = (fft_size / 2) + 1;
	const auto fftsize_inv = multiplier / fft_size;

	// arrays to store fft and audio data
	float timedata[fft_size];
	kiss_fft_cpx freqdata[freqdata_len];
	float buffer[fft_size * sf.channels()];

	// get terminal size, initialize amplitudes array
	TerminalSize tsize;
	std::vector<float> amplitudes(tsize.width);

	const auto logmax = log(freqdata_len);
	const auto sqrtmax = sqrt(freqdata_len);

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
			const auto frames_read = sf.readf(buffer, fft_size);

			// break on end of file
			if (!frames_read)
				break;

			// play the audio as it is read
			pa_stream.write(buffer, frames_read);

			// we can't process anything less than fft_size
			if (frames_read != fft_size)
				break;
		}

		// only consider the last channel (for now)
		for (int i = 0; i < fft_size; ++i)
			timedata[i] = buffer[i * sf.channels()];

		// perform fft: frequency range to amplitude values are stored in freqdata
		kiss_fftr(cfg, timedata, freqdata);

		// zero out array since we are creating sums
		for (auto &a : amplitudes)
			a = 0;

		// group up freqdata into bigger ranges by summing up amplitudes for the same calculated index
		for (auto i = 0; i < freqdata_len; ++i)
		{
			const auto [re, im] = freqdata[i];
			const float amplitude = sqrt((re * re) + (im * im));
			int index;
			switch (scale)
			{
			case Args::Scale::LINEAR:
				index = (float)i / freqdata_len * width;
				break;
			case Args::Scale::LOG:
				index = log(i + 1) / logmax * width;
				break;
			case Args::Scale::SQRT:
				index = sqrt(i) / sqrtmax * width;
				break;
			default:
				throw std::runtime_error("impossible!!!!!!!");
			}
			amplitudes[index] += amplitude;
		}

		// clear the terminal
		std::cout << "\ec";

		if (args.scale != Args::Scale::LINEAR)
			amplitudes = SmoothedAmplitudes(amplitudes);

		// print the spectrum
		for (int i = 0; i < width; ++i)
		{
			// calculate height based on amplitude
			int bar_height = fftsize_inv * amplitudes[i] * height;

			// move cursor to (height, i)
			// remember that (0, 0) in a terminal is the top-left corner, so positive y moves the cursor down.
			std::cout << "\e[" << height << ';' << i << 'f';

			// draw the bar upwards `bar_height` high
			for (int j = 0; j < bar_height; ++j)
				// print character, move cursor up 1, move cursor left 1
				std::cout << spectrum_char << "\e[1A\e[1D";
		}
	}

	// resource cleanup
	kiss_fftr_free(cfg);
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
