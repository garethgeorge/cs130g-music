from math import *
from constants import SAMPLE_RATE
import synth2
import struct
import sounddevice as sd
import numpy as np
sd.default.channels = 1
sd.default.samplerate = SAMPLE_RATE

# import pyaudio
'''
def play(source):
	if source.getDuration() == None:
		raise ValueError("excpected source to have a finite duration")

	print ("playing... " + str(source))

	p = pyaudio.PyAudio()
	#open stream
	stream = p.open(format = p.get_format_from_width(2),
	                channels = 1,
	                rate = int(SAMPLE_RATE),
	                output = True)

	#play stream
	duration = source.getDuration()
	exponent = float((2 ** 15) - 1)
	for index in range(0, int(source.getDuration() * SAMPLE_RATE)):
		sample = (source.sample(index / SAMPLE_RATE)) * exponent
		if sample >= exponent:
			sample = exponent
		elif sample <= -exponent:
			sample = -exponent

		packed_value = struct.pack('h', int(sample))
		stream.write(packed_value)
		index = index + 1

	#stop stream
	stream.stop_stream()
	stream.close()

	#close PyAudio
	p.terminate()
'''
def render(source):
	print("rendering audio...")
	if source.getDuration() >= 10000000:
		raise ValueError("excpected source to have a finite duration")
	print(source)

	array = np.zeros(int(source.getDuration() * SAMPLE_RATE), dtype='float32')

	sampleCount = float(len(array))
	print("\tsample count: " + str(sampleCount))
	for sample in range(0, len(array)):
		if sample % SAMPLE_RATE == 0:
			print("\t\t - progress: " + str(sample))
		time = float(sample) / SAMPLE_RATE
		array[sample] = float(source.sample(time))

	return array

def play(samples):
	print("playing")
	sd.play(samples, blocking=True)
