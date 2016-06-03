import sys
import numbers
from math import *
import copy

class SoundSource:
    def __init__(self):
        self._duration = sys.float_info.max
        self._size = None

    def sample(self, time):
        return 0

    def maxAmp(self, time):
        return 0

    def getDuration(self):
        return self._duration

    def setDuration(self, duration):
        self._duration = duration
        self.fixdurations()
        return self

    # used for placing sounds when using the stream operator
    def getSize(self):
        return self._size != None and self._size or self.getDuration()

    def setSize(self, size):
        self._size = size
        return self

    def fixdurations(self):
        pass

    def isDone(self, time):
        return time > self._duration

    def __mul__(self, other):
        if isinstance(other, numbers.Number):
            return SignalScaler(self, other)
        return SignalMult() * self * other

    def __add__(self, other):
        return SignalAdd() + self + other

    def __lshift__(self, other):
        seq = NoteSequence()
        print seq
        return seq << self << other

class SinWave(SoundSource):
    def __init__(self, freq):
        SoundSource.__init__(self)
        self._freq = freq

    def sample(self, time):
        return cos(2 * pi * time * self._freq)

    def maxAmp(self, time):
        return 1
    def __str__(self):
        return "SinWave(" + str(self._freq) + ")"

class Value(SoundSource):
    def __init__(self, value):
        SoundSource.__init__(self)
        self._value = value
    def maxAmp(self, time):
        return self._value
    def sample(self, time):
        return self._value
    def __str__(self):
        return str(self._value)


class SignalAdd(SoundSource):
    def __init__(self):
        SoundSource.__init__(self)
        self._waves = []

    def maxAmp(self, time):
        total = 0
        for wave in self._waves:
            if wave.getDuration() < time:
                break
            total += wave.maxAmp(time)
        return total

    def sample(self, time):
        total = 0
        for wave in self._waves:
            if wave.getDuration() < time:
                break
            total += wave.sample(time)
        return total

    def fixdurations(self):
        for wave in self._waves:
            wave.setDuration(min(wave.getDuration(), self.getDuration()))

    def __str__(self):
        return "(" + " + ".join([str(w) for w in self._waves]) + ")"

    def __add__(self, other):
        if len(self._waves) == 0:
            self.setDuration(other.getDuration())
        else:
            self.setDuration(max(self.getDuration(), other.getDuration()))

        self._waves.append(copy.deepcopy(other))
        self._waves.sort(key=lambda x: -x.getDuration())

        return self

class SignalMult(SoundSource):
    def __init__(self):
        SoundSource.__init__(self)
        self._waves = []

    def maxAmp(self, time):
        total = 1
        for wave in self._waves:
            if wave.getDuration() < time:
                break
            total *= wave.maxAmp(time)
        return total

    def sample(self, time):
        total = 1
        for wave in self._waves:
            if wave.getDuration() < time:
                break
            total *= wave.sample(time)
        return total

    def fixdurations(self):
        for wave in self._waves:
            wave.setDuration(min(wave.getDuration(), self.getDuration()))

    def __str__(self):
        return "(" + " * ".join([str(w) for w in self._waves]) + ")"

    def __mul__(self, other):
        if len(self._waves) == 0:
            self.setDuration(other.getDuration())
        else:
            self.setDuration(max(self.getDuration(), other.getDuration()))

        self._waves.append(copy.deepcopy(other))
        self._waves.sort(key=lambda x: -x.getDuration())

        return self

class SignalScaler(SoundSource):
    def __init__(self, wave, scaler):
        SoundSource.__init__(self)
        self._wave = copy.deepcopy(wave)
        self._scaler = scaler
        self.setDuration(wave.getDuration())

    def fixdurations(self):
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration()))
    def maxAmp(self, time):
        return self._wave.maxAmp(time) * self._scaler

    def sample(self, time):
        return self._wave.sample(time) * self._scaler

    def __str__(self):
        return "(" + str(self._wave) + " * " + str(self._scaler) + ")"

class PhaseShift(SoundSource):
    def __init__(self, wave, offset):
        SoundSource.__init__(self)
        self._offset = offset
        self._wave = copy.deepcopy(wave)
        self.setDuration(self._offset + self._wave.getDuration())

    def sample(self, time):
        if time < self._offset: return 0
        return self._wave.sample(time - self._offset)

    def maxAmp(self, time):
        if time < self._offset: return 0
        return self._wave.maxAmp(time - self._offset)

    def fixdurations(self):
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration() - self._offset))

    def __str__(self):
        return "(" + str(self._wave) + "->" + str(self._offset) + ")"

class Envelope(SoundSource):
    def __init__(self, wave, time):
        SoundSource.__init__(self)
        self._wave = wave
        self._time = float(time)
        self.setDuration(self._wave.getDuration())

    def maxAmp(self, time):
        return self._wave.maxAmp(time)

    def sample(self, time):
        if time < self._time:
            return time / self._time * self._wave.sample(time)
        elif time > self.getDuration() - self._time:
            return (self.getDuration() - time) / self._time * self._wave.sample(time)
        return self._wave.sample(time)

    def fixdurations(self):
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration()))

    def __str__(self):
        return "Envelope(" + str(self._wave) + ", " + str(self._time) + ")"

class Normalize(SoundSource):
    def __init__(self, wave):
        SoundSource.__init__(self)
        self._wave = copy.deepcopy(wave)
        self.setDuration(wave.getDuration())

    def sample(self, time):
        return self._wave.sample(time) / (self._wave.maxAmp(time) or 1)

    def maxAmp(self, time):
        return 1

    def fixdurations(self):
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration()))

    def __str__(self):
        return "Normalize(" + str(self._wave) + ")"

def duration(wave, dur):
    return copy.deepcopy(wave).setDuration(dur)

def harmonics(freq, harmonicsAndStrengths):
    adder = SignalAdd()
    finalMultiple = 1.0 / sum([float(x[1]) for x in harmonicsAndStrengths])
    for multiple, ratio in harmonicsAndStrengths:
        adder += SinWave(freq * multiple) * Value(ratio)
    return adder * Value(finalMultiple)


class LinearDecay(SoundSource):
    def __init__(self, wave):
        SoundSource.__init__(self)
        self._wave = copy.deepcopy(wave)
        self._rate = 1
        self.setDuration(wave.getDuration())

    def fixdurations(self):
        self._rate = -(1.0 / float(self.getDuration()))
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration()))

    def getSize(self):
        return self.getDuration() * 0.5

    def sample(self, time):
        return self._wave.sample(time) * (1.0 + self._rate * time)

    def maxAmp(self, time):
        return self._wave.maxAmp(time)

    def __str__(self):
        return "Decay(" + str(self._wave) + ")"

class NoteSequence(SoundSource):
    def __init__(self):
        SoundSource.__init__(self)
        self._waves = []
        self._sound = Value(0)

    def fixdurations(self):
        # assert(self.getDuration() >= self._sound.getDuration())
        pass

    def maxAmp(self, time):
        return self._sound.maxAmp(time)

    def sample(self, time):
        return self._sound.sample(time)

    def __lshift__(self, other):
        def reduceNoteList(thing):
            if len(thing) == 1:
                return thing[0]
            if len(thing) == 0:
                return duration(Value(0), 0)

            half = len(thing)/2
            a = reduceNoteList(thing[:half])
            b = reduceNoteList(thing[half:])
            return (a + PhaseShift(b, a.getSize())).setSize(a.getSize() + b.getSize())

        self._waves.append(copy.deepcopy(other))
        self._sound = reduceNoteList(self._waves)
        self.setDuration(self._sound.getDuration())
        return self

    def __str__(self):
        return "(" + " << ".join([str(x) for x in self._waves]) + ")"

class ExponentialDecay(SoundSource):
    def __init__(self, wave, reducedBy):
        SoundSource.__init__(self)
        self._wave = copy.deepcopy(wave)
        self._reducedBy = reducedBy
        self.setDuration(wave.getDuration() * 1.5)

    def fixdurations(self):
        self._exp = self._reducedBy ** (1.0 / self.getDuration())
        self._wave.setDuration(min(self._wave.getDuration(), self.getDuration()))

    def getSize(self):
        return self.getDuration() * 0.5

    def sample(self, time):
        return self._wave.sample(time) * (self._exp ** time)

    def maxAmp(self, time):
        return self._wave.maxAmp(time)

    def __str__(self):
        return "Decay(" + str(self._wave) + ")"
