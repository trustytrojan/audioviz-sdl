#pragma once

#include <fftw3.h>

class FftwDftR2c1d
{
	int N;
	double *const in = (double *)fftw_malloc(sizeof(double) * N);
	fftw_complex *const out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
	const fftw_plan p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

public:
	FftwDftR2c1d(int N) : N(N) {}

	~FftwDftR2c1d()
	{
		fftw_destroy_plan(p);
		fftw_free(in);
		fftw_free(out);
	}

	const fftw_complex *execute(const float *const inputdata)
	{
		for (int i = 0; i < N; ++i)
			in[i] = inputdata[i];
		fftw_execute(p);
		return out;
	}
};