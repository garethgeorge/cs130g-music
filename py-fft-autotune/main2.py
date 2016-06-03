# requires PySoundFile 0.8.1
# https://pypi.python.org/pypi/PySoundFile
# pip install PySoundFile
# pip install Pillow
# pip install numpy

import soundfile as sf
import numpy as np
from math import *
# from PIL import Image

BLOCK_SIZE = 2048 * 8 # 2048 * 32

sounddata, samplerate = sf.read("dirtypaws.wav")
channel = sounddata.transpose()[0][0:samplerate * 60]

totalSamples = len(channel)

frames = int(totalSamples / BLOCK_SIZE)
fftPerFrame = 16
fftPerFrameDiv = 1.0 / float(fftPerFrame)

outputdata = np.zeros((frames) * BLOCK_SIZE, dtype=float)

def hannWindow(n, N):
	return 0.5 * (1 - cos((2 * pi * n) / (N - 1)))
def zipMult(list1, list2):
	return [a * b for a, b in zip(list1, list2)]

for x in range(0, (frames - 1) * fftPerFrame):
    start_sample = int((x * fftPerFrameDiv) * BLOCK_SIZE)
    end_sample = int(start_sample + BLOCK_SIZE)
    print str(x) + " : " + str(start_sample) + " - " + str(end_sample)
    samples = channel[start_sample:end_sample]
    fft = np.fft.fft(samples)

    if len(fft) < BLOCK_SIZE * 0.5: break

    #for x in range(0, BLOCK_SIZE):
    #    fft[x] = fft[x] * float(BLOCK_SIZE - x) / float(BLOCK_SIZE)

    fftHalf = fft[BLOCK_SIZE // 2:BLOCK_SIZE]
    fftHalf = np.concatenate(([0] * 100, fftHalf[:-100]))
    fftHalfA = fftHalf[:-1]
    fft = np.concatenate((fftHalfA, fftHalf))


    ifft = np.fft.ifft(fft)
    for x in range(0, len(ifft)):
        outputdata[start_sample + x] += abs(ifft[x]) * fftPerFrameDiv

sf.write('newfile.wav', outputdata, samplerate)
