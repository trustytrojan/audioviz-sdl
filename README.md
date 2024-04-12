# audioviz
my own audio visualizer, because after effects sucks.<br>
for right now, audioviz only supports linux, but with the libraries i'm using, it shouldn't be hard to port to windows.<br>
here's a demo: https://youtu.be/0f_oSrnT6j0

the default parameters construct a spectrum inspired by [Spectrolizer](https://spectrolizer.aicore-software.com/)

## building
1. install required [dependencies](#dependencies)
2. run `make`
3. optionally install with `sudo make install`, or `make install` if you are `root`

## dependencies
- [libsndfile](https://github.com/libsndfile/libsndfile)
- [FFTW](https://fftw.org)
- [PortAudio](https://github.com/PortAudio/portaudio)
- [p-ranav/argparse](https://github.com/p-ranav/argparse)
- [SDL2pp](https://github.com/libSDL2pp/libSDL2pp)
- [ferzkopp/SDL2_gfx](https://github.com/ferzkopp/SDL2_gfx)
