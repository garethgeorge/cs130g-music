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


