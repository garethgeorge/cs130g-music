#include <iostream>
#include "synth2.h"
#include "fileformats.h"


using namespace synth;
auto overtones(int f) {
	return normalize(SinWave(freq(f)) * ConstValue(4) + 
		   SinWave(freq(f * 3)) * ConstValue(3) + 
		   SinWave(freq(f * 2)) * ConstValue(2) + 
		   SinWave(freq(f * 4)));
}

int main() {
	synth::WavFileWriter test("test.wav");

	auto a = finiteOf(finiteOf(envelopeOf(overtones(220)), seconds(0.25)), seconds(0.4));
	auto b = finiteOf(finiteOf(envelopeOf(overtones(440)), seconds(0.25)), seconds(0.4));
	auto c = finiteOf(finiteOf(envelopeOf(overtones(880)), seconds(0.25)), seconds(0.4));

	auto sound = a << b << c << a << b << c << c << a << b;
	auto sound2 = sound * ConstValue(0.2);
	test.writeHeader();
	test.render(sound);
	test.close();

}

