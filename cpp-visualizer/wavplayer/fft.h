
/*

import cmath
import numpy as np

# https://jeremykun.com/2012/07/18/the-fast-fourier-transform/

# note that omega is that expression we factored out in our derivation before!!!
def omega(p, q):
	return cmath.exp((2.0 * cmath.pi * 1j * q) / p)

def fft(signal):
	n = len(signal)
	if n == 1:
		return signal # nothing to do here
	else:
		# we take an even and an odd fft
		evenFft = fft(signal[::2])
		oddFft = fft(signal[1::2])

		combined = [0] * n
		for m in range(0, n / 2):
			# we then combine the even and odds getting two fft values on a longer range per pair of values
			# in the even and odd ffts
			combined[m] = evenFft[m] + omega(n, -m) * oddFft[m]
			combined[m + n / 2] = evenFft[m] - omega(n, -m) * oddFft[m]
		return combined
*/

#include <complex>
#include <iostream>
#include <algorithm>

const double pi = std::acos(-1);

typedef std::complex<double> Complex;

Complex omega(double p, double q) {
	return std::exp((2.0 * pi * Complex(0, 1) * q) / p);
}

template<int sampleCount>
struct FFT {
	static void run(Complex* samples, Complex* results) {
		Complex evenSamples[sampleCount / 2];
		Complex oddSamples[sampleCount / 2];
		for (int i = 0; i < sampleCount / 2.0; i++) {
			evenSamples[i] = samples[i * 2];
			oddSamples[i] = samples[i * 2 + 1];
		}

		Complex evenFft[sampleCount / 2];
		Complex oddFft[sampleCount / 2];

		FFT<sampleCount/2>::run(evenSamples, evenFft);
		FFT<sampleCount/2>::run(oddSamples, oddFft);

		for (int m = 0; m < sampleCount / 2; ++m) {
			results[m] = evenFft[m] + omega(sampleCount, -m) * oddFft[m];
			results[m + sampleCount / 2] = evenFft[m] - omega(sampleCount, -m) * oddFft[m];
		}
	}
};

template<>
struct FFT<1> {
	static void run(Complex* samples, Complex* results) {
		results[0] = samples[0];
	}
};

// taken from rosetta code
template<int sampleCount>
struct IFFT {
	static void run(Complex* _fft, Complex* result) {
		Complex fft[sampleCount];
		memcpy(fft, _fft, sizeof(Complex) * sampleCount);

		std::transform(fft, fft + sampleCount, fft, [](Complex val) -> Complex {
			return std::conj(val);
		});

		FFT<sampleCount>::run(fft, fft);

		std::transform(fft, fft + sampleCount, fft, [](Complex val) -> Complex {
			return std::conj(val);
		});

		std::transform(fft, fft + sampleCount, fft, [](Complex val) -> Complex {
			return val / ((double) sampleCount);
		});

		memcpy(result, fft, sizeof(fft));
	}
};

