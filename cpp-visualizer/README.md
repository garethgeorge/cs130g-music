# SFML Simple Sound Scripts
A simple SFML based FFT visualizer I wrote in C++ as well as an FFT visualizer for the system microphone input.

This uses an FFT implementation I coded myself. It's a bit overly templated because I was playing around... if I have time to rewrite I should change this to be more flexible.

# Installation
OSX
```
brew install sfml
brew install boost
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```

# Screenshots
microphone input visualizer
![Alt text](https://raw.githubusercontent.com/garethgeorge/cs130g-music/master/cpp-visualizer/screenshot-1.png?raw=true "Mic-Visualizer")
.wav song visualizer
![Alt text](https://raw.githubusercontent.com/garethgeorge/cs130g-music/master/cpp-visualizer/screenshot-2.png?raw=true"Visualizer")
