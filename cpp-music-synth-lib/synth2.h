#ifndef __SYNTH_H_
#define __SYNTH_H_

#include <cassert>
#include <math.h>
#include <utility>
#include <type_traits>
#include <string>
#include <sstream>
#include <iostream>

namespace synth {

const int SAMPLES_PER_SECOND = 44100;
const float pi = 3.141592653589;
typedef int Time;

/*
	ABSTRACTING THE CONCEPT OF TIMING
*/

inline extern constexpr int seconds(float seconds) {
	return seconds * SAMPLES_PER_SECOND;
}

inline extern constexpr int samples(float samples) {
	return (int) samples;
}

inline extern constexpr int freq(float freq) {
	return (int) ((float) seconds(1) / freq);
}

struct TimeRange {

	Time offset;
	Time duration;

	TimeRange(Time offset, Time duration) : offset(offset), duration(duration) { };

	int getOffset() {
		return offset;
	}

	int getDuration() {
		return duration;
	}
};

inline extern TimeRange operator + (TimeRange& a, TimeRange& b) {
	return TimeRange(a.offset + b.offset, a.duration + b.duration);
}


/*
	SOUND SOURCE
*/
struct Context {
	int time;
	int duration;
	float* samples;
	Context(int time, int duration, float* samples) : time(time), duration(duration), samples(samples) { };
};


struct SoundSourceBase {
	virtual ~SoundSourceBase() { };
	virtual float sample(const Context& context) = 0;
	virtual float maxAmp() const = 0;
	virtual SoundSourceBase* dynamicCopy() const = 0;
	virtual std::string toString() const = 0;
};

template<class Derived>
struct SoundSource : public SoundSourceBase {
	SoundSource() {
		static_assert(std::is_default_constructible<Derived>::value, "Classes deriving from SoundSource must provide default constructors");
		//static_assert(std::is_copy_constructible<Derived>::value, "Classes deriving from SoundSource must provide a copy constructor");
		static_assert(std::is_copy_assignable<Derived>::value, "Classes derviing from SoundSource must be copy assignable");
	}
	virtual ~SoundSource() {

	}

	// get a reference to the derived class
	Derived& get_ref() {
		return static_cast<Derived&>(*this);
	}

	const Derived& get_ref() const {
		return static_cast<const Derived&>(*this);
	}
	
	float sample(const Context& context) {
		return get_ref()._sample(context);
	}

	float _sample(const Context& context) {
		return 0;
	}
	
	float maxAmp() const {
		return get_ref()._maxAmp();
	}

	float _maxAmp() const {
		return 0;
	}

	// assignment operator using the inherit method that all derived classes must implement
	template<typename T>
	Derived& operator = (const SoundSource<T>& other) {
		dynamic_cast<Derived&>(*this).inherit(other.get_ref());
		return *this;
	}

	// create a copy of this class on the heap
	SoundSourceBase* dynamicCopy() const {
		return new Derived(get_ref());
	}

	std::string toString() const {
		return dynamic_cast<const Derived&>(*this)._toString();
	}
};

// a SinWave
// usege: SinWave(freq(220))
struct SinWave : public SoundSource<SinWave> {
	float period;
	
	SinWave() { };
	SinWave(Time period) : period(period) { };
	SinWave(const SinWave& parent) {
		*this = parent;
	}
	
	float _sample(const Context& context) {
		return sin(2.0 * pi * ((float) context.time) / period);
	}

	constexpr float _maxAmp() const {
		return 1;
	}

	std::string _toString() const {
		std::stringstream ss;
		ss << "SinWave(" << period << ")";
		return ss.str();
	}
	
	void inherit(const SinWave& other) {
		period = other.period;
	}
};

// a constant value
// usage: ConstValue(float a)
struct ConstValue : public SoundSource<ConstValue> {
	float value;
	
	ConstValue() { };
	ConstValue(float value) : value(value) { };
	ConstValue(const ConstValue& other) { *this = other; };
	
	float _maxAmp() const {
		return value;
	}

	float _sample(const Context&) {
		return value;
	}

	std::string _toString() const {
		std::stringstream ss;
		ss << "ConstValue(" << value << ")";
		return ss.str();
	}

	void inherit(const ConstValue& parent) {
		value = parent.value;
	}

};

// a Dynamic Sound Source
struct Dynamic : public SoundSource<Dynamic> {
	SoundSourceBase* base;

	Dynamic() { base = nullptr; };

	Dynamic(const Dynamic& parent) {
		base = parent.base->dynamicCopy();
	}

	template<class T>
	Dynamic(const SoundSource<T>& base) {
		this->base = base.dynamicCopy();
	}

	~Dynamic() {
		// there is a memory leak here!
		base = nullptr;
	}

	float _maxAmp() const {
		return base->maxAmp();
	}

	float _sample(const Context& context) {
		return base->sample(context);
	}

	std::string _toString() const {
		return std::string("Dynamic");
	}

	void inherit(const Dynamic& other) {
		if (this->base != nullptr)
			delete this->base;
		this->base = other.base->dynamicCopy();
	}
};

// wave adder
// usage: waveAdd(a, b, c, d, e) where variables are waves
template<class T1, class... T2>
struct WaveAdder : public SoundSource<WaveAdder<T1, T2...>> {
	T1 wave1;
	WaveAdder<T2...> wave2;
	
	WaveAdder() { };

	WaveAdder(const T1& wave1, const T2... theRest) : wave2(theRest...) {
		static_assert(std::is_base_of<SoundSource<T1>, T1>::value, "WaveAdder can only add objects derived from SoundSource");
	
		this->wave1 = wave1;
	}

	float _maxAmp() const {
		return wave1.maxAmp() + wave2.maxAmp();
	}

	float _sample(const Context& context) {
		return wave1.sample(context) + wave2.sample(context);
	}

	std::string _toString() const {
		return std::string("WaveAdder(") + wave1.toString() + ", " + wave2.toString();
	}

	void inherit(const WaveAdder<T1, T2...>& parent) {
		wave1 = parent.wave1;
		wave2 = parent.wave2;
	}
};

template<class T1>
struct WaveAdder<T1> : public SoundSource<WaveAdder<T1>> {
	T1 wave1;
	
	WaveAdder() { };

	WaveAdder(const T1& wave1) {
		static_assert(std::is_base_of<SoundSource<T1>, T1>::value, "WaveAdder can only add objects derived from SoundSource");
		
		this->wave1 = wave1;
	}

	float _maxAmp() const {
		return wave1.maxAmp();
	}

	float _sample(const Context& context) {
		return wave1.sample(context);
	}

	std::string _toString() const {
		return wave1.toString() + ")";
	}

	void inherit(const WaveAdder<T1>& parent) {
		wave1 = parent.wave1;
	}
};

// wave multiplier
// usage: waveMult(a, b, c, d, e, f)
template<class T1, class... T2>
struct WaveMult : public SoundSource<WaveMult<T1, T2...>> {
	T1 wave1;
	WaveMult<T2...> wave2;
	
	WaveMult() { };

	WaveMult(const T1& wave1, const T2... theRest) : wave2(theRest...) {
		static_assert(std::is_base_of<SoundSource<T1>, T1>::value, "WaveMult can only multiply objects derived from SoundSource");
	
		this->wave1 = wave1;
	}

	float _maxAmp() const {
		return wave1.maxAmp() * wave2.maxAmp();
	}

	float _sample(const Context& context) {
		return wave1.sample(context) * wave2.sample(context);
	}

	std::string _toString() const {
		return std::string("WaveMult(") + wave1.toString() + ", " + wave2.toString();
	}

	void inherit(const WaveMult<T1, T2...>& parent) {
		wave1 = parent.wave1;
		wave2 = parent.wave2;
	}
};

template<class T1>
struct WaveMult<T1> : public SoundSource<WaveMult<T1>> {
	T1 wave1;
	
	WaveMult() { };

	WaveMult(const T1& wave1) {
		static_assert(std::is_base_of<SoundSource<T1>, T1>::value, "WaveMult can only multiply objects derived from SoundSource");
		
		this->wave1 = wave1;
	}

	float _maxAmp() const {
		return wave1.maxAmp();
	}

	float _sample(const Context& context) {
		return wave1.sample(context);
	}

	std::string _toString() const {
		return wave1.toString() + ")";
	}

	void inherit(const WaveMult<T1>& parent) {
		wave1 = parent.wave1;
	}
};

// Finite range from 0 to x of a sound
// usage: finiteOf(soundSource, duration)
template<class DerivedClass>
struct Finite : public SoundSource<Finite<DerivedClass>> {
	Time duration;
	DerivedClass wave;
	
	Finite() {
		static_assert(std::is_base_of<SoundSource<DerivedClass>, DerivedClass>::value, "Finite expects to be provided with a SoundSource.");
	};

	Finite(const DerivedClass& wave, Time duration) : wave(wave), duration(duration) {
	}

	float _maxAmp() const {
		return this->wave.maxAmp();
	}

	float _sample(const Context& context) {
		if (context.time >= duration)
			return 0;
		return wave.sample(context);
	}

	float getDuration() const {
		return duration;
	}

	std::string _toString() const {
		std::stringstream ss;
		ss << "(" << wave.toString() << "[" << duration << "])";
		return ss.str();
	}

	void inherit(const Finite<DerivedClass>& parent) {
		this->duration = parent.duration;
		this->wave = parent.wave;
	}
};

// phase shifts of a given wave
// usage: shiftOf(wave, shiftBySamples);
template<class DerivedClass>
struct PhaseShift : public SoundSource<PhaseShift<DerivedClass>> {
	Time shift;
	DerivedClass wave;
	
	PhaseShift() {
		static_assert(std::is_base_of<SoundSource<DerivedClass>, DerivedClass>::value, "Finite expects to be provided with a SoundSource.");
	};

	PhaseShift(const DerivedClass& wave, Time shift) : wave(wave), shift(shift) {
	}

	float _maxAmp() const {
		return this->wave.maxAmp();
	}

	float _sample(const Context& context) {
		if (context.time < shift)
			return 0;
		Context c(context.time - shift, context.duration - shift, context.samples + shift);
		return wave.sample(c);
	}

	float getShiftAmount() const {
		return shift;
	}

	std::string _toString() const {
		std::stringstream ss;
		ss << "(" << shift << ">>" << wave.toString() << ")";
		return ss.str();
	}

	void inherit(const Finite<DerivedClass>& parent) {
		this->shift = parent.shift;
		this->wave = parent.wave;
	}
};

/*
	utilities
*/
template<class T>
Finite<T> finiteOf(const SoundSource<T>& parent, Time time) {
	return Finite<T>(parent.get_ref(), time);
}

template<class T>
PhaseShift<T> shiftOf(const SoundSource<T>& parent, Time shift) {
	return PhaseShift<T>(parent.get_ref(), shift);
}

template<class... T>
auto waveMult(T... args) {
	return WaveMult<T...>(args...);
}

template<class... T>
auto waveAdd(T... args) {
	return WaveAdder<T...>(args...);
}

template<class T>
auto dynamicOf(const Finite<T>& wave) {
	return finiteOf(Dynamic(wave.wave), wave.getDuration());
}

template<class T>
auto dynamicOf(const SoundSource<T>& wave) {
	return Dynamic(wave.get_ref().wave);
}


/*
	wave addition
*/
// infinite
template<class T1, class T2>
auto operator + (const Finite<T1>& wave1, const Finite<T2>& wave2) -> Finite<decltype(waveAdd(wave1, wave2))> {
	return finiteOf(waveAdd(wave1, wave2), std::max(wave1.getDuration(), wave2.getDuration()));
}

// finite
template<class T1, class T2>
WaveAdder<T1, T2> operator + (const SoundSource<T1>& wave1, const SoundSource<T2>& wave2) {
	return WaveAdder<T1, T2>(wave1.get_ref(), wave2.get_ref());
}

/*
	wave multiplication
*/
// finite
template<class T1, class T2>
auto operator * (const Finite<T1>& wave1, const Finite<T2>& wave2) -> Finite<decltype(waveMult(wave1, wave2))> {
	return finiteOf(waveMult(wave1, wave2), std::min(wave1.getDuration(), wave2.getDuration()));
}

template<class T1, class T2>
auto operator * (const Finite<T1>& wave1, const SoundSource<T2>& wave2) -> Finite<decltype(wave1.wave * wave2.get_ref())> {
	return finiteOf(wave1.wave * wave2.get_ref(), wave1.getDuration());
}

// infinite
template<class T1, class T2>
WaveMult<T1, T2> operator * (const SoundSource<T1>& wave1, const SoundSource<T2>& wave2) {
	return WaveMult<T1, T2>(wave1.get_ref(), wave2.get_ref());
}


/*
	wave normalization
*/
template<class T>
auto normalize(const Finite<T>& wave) -> Finite<decltype(normalize(wave.wave))> {
	return finiteOf(normalize(wave.wave), wave.getDuration());
}


template<class T>
WaveMult<T, ConstValue> normalize(const SoundSource<T>& wave) {
	return wave.get_ref() * ConstValue(1.0 / wave.maxAmp());
}

template<class T1, class T2>
auto operator << (const Finite<T1>& wave1, const Finite<T2>& wave2) {
	auto sum = wave1 + shiftOf(wave2, wave1.getDuration());
	int length = wave1.getDuration() + wave2.getDuration();
	return Finite<decltype(sum)>(sum, length);
}


/*
	EFFECTS
*/
template<typename T> 
T abs(T v) {
	return v < 0 ? -v : v;
}

template<class WaveType>
struct Envelope : public SoundSource<Envelope<WaveType>> {
	WaveType wave;

	const static constexpr float duration = seconds(0.1);
	
	Envelope() { };
	Envelope(const SoundSource<WaveType>& wave) {
		this->wave.inherit(wave.get_ref());
	}

	float _maxAmp() const {
		return wave.maxAmp();
	}

	float _sample(const Context& context) {
		float val = wave.sample(context);
		std::cout << context.time << ":" << context.duration << std::endl;
		if (context.time < duration) {
			std::cout << "ENVELOPE!" << std::endl;
			return (context.time / duration) * val;
		}
		else if (context.time > context.duration - duration) {
			std::cout << "ENVELOPE!" << std::endl;
			return ((context.duration - context.time) / duration) * val;
		}
		return val;
	}

	void inherit(const Envelope<WaveType>& other) {
		this->wave.inherit(other.wave);
	}

	std::string _toString() const {
		return std::string("Envelope(") + wave.toString() + ")";
	}
};

template<class T>
auto envelopeOf(const Finite<T>& wave) {
	return finiteOf(Envelope<T>(wave), wave.getDuration());
}

template<class T>
auto envelopeOf(const SoundSource<T>& wave) {
	return Envelope<T>(wave.get_ref());
}

}

#endif
