# Python Music Synth
by Gareth George

written in python this library provides a simple way of working with various sin waves and synthetic instruments. Provided is a basic sin wave class with overloaded addition, multiplication and << operators.

 - multiplication allows you to multiply any two waves values.
 - addition allows you to add any two waves
 - phase shift lets you phase shift a wave (used by << operator to lay out sequences of sounds)
 - Finite(wave) class that indicates that a wave is finite in it's duration. used to express the idea of taking a window of wave for the actual output.
