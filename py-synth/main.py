from out import *
from synth2 import *
from instruments import *

freqA = 220.00
freqB = 220.00 * 9.00/8.00
freqC = 220.00 * 4.00/3.00
freqD = 220.00 * 3.00/2.00
freqE = 220.00 * 2.00

A = duration(ExponentialDecay(Envelope(harmonics(freqA, [(1, 1), (2, 0.2), (4, 0.05)]), 0.1), 0.0001), 0.5)
B = duration(ExponentialDecay(Envelope(harmonics(freqB, [(1, 1), (2, 0.2), (4, 0.05)]), 0.1), 0.0001), 0.5)
C = duration(ExponentialDecay(Envelope(harmonics(freqC, [(1, 1), (2, 0.2), (4, 0.05)]), 0.1), 0.0001), 0.5)
D = duration(ExponentialDecay(Envelope(harmonics(freqD, [(1, 1), (2, 0.2), (4, 0.05)]), 0.1), 0.0001), 0.5)
E = duration(ExponentialDecay(Envelope(harmonics(freqE, [(1, 1), (2, 0.2), (4, 0.05)]), 0.1), 0.0001), 0.5)

Ag = duration(Envelope(stringPlucked(freqA), 0.06), 0.65)
Bg = duration(Envelope(stringPlucked(freqB), 0.06), 0.65)
Cg = duration(Envelope(stringPlucked(freqC), 0.06), 0.65)
Dg = duration(Envelope(stringPlucked(freqD), 0.06), 0.65)
Eg = duration(Envelope(stringPlucked(freqE), 0.06), 0.65)
A = Ag.setSize(0.65)
B = Bg.setSize(0.65)
C = Cg.setSize(0.65)
D = Dg.setSize(0.65)
E = Eg.setSize(0.65)


notesIntro = E << B << B << B << C << B << B << B << E << B << B << A << B << C << D << E # falling off the cliff
notesMidSequence1 = A << B << A << A << B << A << A << A << B << A << B << C << A << B << C << E << D << C << B << B << A << C << B
notesMidSequence2 = B << A << A << B << D << D << B << C << C << B << E << E << B << A << A << B << B << B << B
notesMidSequence1 = notesMidSequence1 + PhaseShift(notesMidSequence1, 0.4) * 0.1
notesMidSequence2 = notesMidSequence1 + PhaseShift(notesMidSequence1, 0.3) * 0.1
notesEnd = E << D << C << B << A << A << B << B << D << D << C << C << E << E << A << A << C << C << B << B << A << B << B << C << B << B << D << B << B << A << B << A << LinearDecay(B << B << B << B << B)

# guitarSolo = Ag << Bg << Cg << Dg << Cg << Dg << Eg

notes = notesIntro << notesMidSequence1 << notesMidSequence2 << notesEnd
# notes = guitarSolo


play(render(Normalize(notes) * 0.25))
