#include <cmath>
#include "fileformats.h"
#include <iostream>

namespace synth {

	/*
		wav files
	 */
	WavFileWriter::WavFileWriter(const char* fname) {
		sampleRate = 44100;
		numChannels = 1;
		bitsPerSample = 16;

		maxAmplitude = 32760;

		f.open(fname, std::ios::binary);
	}

	WavFileWriter::~WavFileWriter() {
		if (f.is_open())
			close();
	}

	void WavFileWriter::close() {
		size_t file_length = f.tellp();

		f.seekp(data_chunk_pos + 4);
		writeBytes(file_length - data_chunk_pos + 8);

		f.seekp(0 + 4);
		writeBytes(file_length - 8, 4 ); 

		f.close();
	}

	void WavFileWriter::writeHeader() {
		// we leave 4 bytes with ---- so we have a place to fill in the size at the end!
		f.write("RIFF    WAVEfmt ", 4 * 4);
		
		writeBytes(16, 4); // Subchunk1Size: sub chunk size: 16 for pcm, size of the rest of the subchunk that follows this number
		writeBytes(1, 2); // AudioFormat: PCM = 1 linear quantization - indicates no compression
		writeBytes(numChannels, 2); // NumChannels:  1 = mono.
		writeBytes(sampleRate, 4); // sample rate.
		writeBytes(numChannels * sampleRate * bitsPerSample / 8, 4); // ByteRate
		writeBytes(numChannels * bitsPerSample / 8, 2); // BlockAlign
		writeBytes(bitsPerSample, 2); // BitsPerSample

		data_chunk_pos = f.tellp();
		f.write("data    ", 4 * 2); // (chunk size to be filled in later)
	}	

	void WavFileWriter::writeSample(double sample) {
		int value = (int) (sample * ((double) maxAmplitude));
		if (value >= maxAmplitude) 
			value = maxAmplitude - 1;
		if (value <= -maxAmplitude)
			value = -maxAmplitude + 1;
		writeBytes(value, 2);
	}

	void WavFileWriter::writeBytes(int value, unsigned size) {
		for (; size; --size, value >>= 8)
			f.put(static_cast <char> (value & 0xFF));
	}
	
}