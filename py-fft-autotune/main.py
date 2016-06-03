# requires PySoundFile 0.8.1
# https://pypi.python.org/pypi/PySoundFile
# pip install PySoundFile
# pip install Pillow
# pip install numpy

import soundfile as sf
import numpy as np
from math import *
from PIL import Image

BLOCK_SIZE = 2048 * 2

# read the sound file
sounddata, samplerate = sf.read("dirtypaws.wav")
channel = sounddata.transpose()[0]
channel = channel[0:samplerate * 60]

# setup for producing the image
imageH = int(len(channel) / BLOCK_SIZE)
imgDataNew = np.zeros((imageH, BLOCK_SIZE, 3), dtype=np.uint8)

# output data
frameCount = int(len(channel) / BLOCK_SIZE)
outputSample = np.zeros(frameCount * BLOCK_SIZE, dtype=float)

# window function
def hannWindow(n, N):
	return 0.5 * (1 - cos((2 * pi * n) / (N - 1)))

def zipMult(list1, list2):
	return [a * b for a, b in zip(list1, list2)]
windowArray = [hannWindow(x, BLOCK_SIZE) for x in range(0, BLOCK_SIZE)]

for x in range(0, frameCount):
	# take a window of the samples
	samples_startIndex = (x * BLOCK_SIZE)
	samples_endIndex = (samples_startIndex + BLOCK_SIZE)
	samples = channel[samples_startIndex:samples_endIndex]

	if len(samples) < BLOCK_SIZE:
		break ;

	# take the fft
	fft = np.fft.fft(samples)
	fftHalf = fft[0:(len(fft)/2)]
	print "len samples: ", len(samples), " fft: ", len(fft)
	print "fft half len: ", len(fftHalf)

	# apply transformation
	fftHalf = np.concatenate(([0] * 20, fftHalf[:-20]))

	# write the output samples
	mirrored = np.concatenate((fftHalf, fftHalf[::-1]))
	frequencyDomain = np.fft.ifft(mirrored)

	# update image after
	imgDataNew[x] = [[int(abs(sample) * 20), int(abs(sample) * 50), int(abs(sample) * 50)] for sample in frequencyDomain]

	for x in range(0, len(frequencyDomain)):
		outputSample[samples_startIndex + x] += abs(frequencyDomain[x])

# img = Image.fromarray(imgDataOrig, 'RGB')
# img.save('image-old.png')

img = Image.fromarray(imgDataNew, 'RGB')
img.save('image-new.png')

sf.write('newfile.wav', outputSample, samplerate)
