from synth2 import *
from out import *
import numpy as np
from constants import SAMPLE_RATE

class RingBuffer:
	def __init__(self, size):
		self._used = 0
		self._size = size
		self._data = np.zeros(size, dtype=float)
		self._last = 0
		self._first = 0

	def enqueue(self, value):
		assert(not self.isFull())

		self._used = self._used + 1
		self._data[self._last] = value
		self._last = self._last + 1
		if self._last >= self._size:
			self._last = 0

	def dequeue(self):
		self._used = self._used - 1
		val = self._data[self._first]
		self._first = self._first + 1
		if self._first >= self._size:
			self._first = 0
		return val

	def peek(self):
		return self._data[self._first]

	def size(self):
		return self._used

	def isFull(self):
		return self._used == self._size


class KarplusStrong(SoundSource):
	# see https://www.cs.princeton.edu/courses/archive/fall07/cos126/assignments/guitar.html
	# for other interesting filters see: http://crypto.stanford.edu/~blynn/sound/digital.html
	def __init__(self, frequency, energyDecay):
		SoundSource.__init__(self)

		self._N = int(SAMPLE_RATE / frequency)
		self._buff = RingBuffer(self._N)
		self._energyDecay = energyDecay # good value is 0.996

		self.pluckRandom()

	def tic(self):
		firstSample = self._buff.dequeue()
		nextSample = self._buff.peek()

		avg = (firstSample + nextSample) * 0.5 * self._energyDecay
		self._buff.enqueue(avg)

	def pluckRandom(self):
		# empty the buffer
		while self._buff.size() > 0:
			self._buff.dequeue()

		# "pluck" by putting random samples into the ring buffer
		# 	a guitar string pluc is complex and can contain any frequencies
		# 	so initializing with random noise is correct
		for x in np.random.uniform(1, -1, self._N):
			self._buff.enqueue(x)

		return self

	def pluckTriangleWave(self):
		# empty the buffer
		while self._buff.size() > 0:
			self._buff.dequeue()

		# "pluck with a single pluse on a triangle wave"
		for i in range(0, int(self._N * 0.5)):
			self._buff.enqueue(float(i) / float(self._N * 0.5))
		for i in range(int(self._N * 0.5), self._N):
			self._buff.enqueue(1.0 - float(i) / float(self._N * 0.5))
		return self


	# interesting effects needs to be investigated more
	def pluckSinusoid(self, freq):
		# empty the buffer
		while self._buff.size() > 0:
			self._buff.dequeue()

		for x in range(0, self._N):
			self._buff.enqueue(sin(2 * pi * freq * x))

		return self

	def sample(self, time):
		self.tic() # note: this only works if the function is ACTUALLY sampled at exactly sample rate
		           #       otherwise the function will decay faster or slower than expectd
		return self._buff.peek()

	def maxAmp(self, time):
		return 1

	def __str__(self):
		return str("KarplusStrong(" + str(self._N) + ")")

def stringPlucked(freq):
	# harmonics ratios from Will Brewer
	a = KarplusStrong(freq, 0.999).pluckRandom()
	b = KarplusStrong(freq * 7.0/12, 0.999).pluckRandom() * Value(0.08)
	c = KarplusStrong(freq * 21.0/12.0, 0.999).pluckRandom() * Value(0.02)
	return (a + b + c)
