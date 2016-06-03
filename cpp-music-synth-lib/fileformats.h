#ifndef __WAVEFILE_H_
#define __WAVEFILE_H_

#include <fstream>
#include "synth2.h"

namespace synth {

	class WavFileWriter {
	private:
		std::ofstream f;

		constexpr static double twoPi = 6.28318530;

		size_t data_chunk_pos;
		int sampleRate;
		int numChannels;
		int bitsPerSample;
		int maxAmplitude;

		void writeBytes(int value, unsigned size = 4);

	public:
		WavFileWriter(const char* fname);
		~WavFileWriter();

		void writeHeader();
		void writeSample(double sample);
		void close();

		template<class T>
		void render(Finite<T>& source) {
			int duration = source.getDuration();
			float* array = new float[duration];
			Context c(0, duration, array);
			for (int i = 0; i < source.getDuration(); ++i) {
				c.time = i;
				writeSample(source.sample(c));
			}
			delete[] array;
		}
	};

	/*
		TODO: tutorial on how to actually use this for nonstandard configurations.

		TUTORIAL:
		default sample rate is 44100
		default numChannels is 1 since it's mono.
		
		you must begin by callind writeHeader once you have finished setting up settings!
		you must take (sample rate) samples per second and use writeSample to write them to the file.
		you must close the file with the close function to finish writing headers and properly close the file.
	*/
};

#endif