CC = g++
CFLAGS = -Wall -Wextra -g
INCLUDE = -I/usr/local/include/kissfft
LDLIBS = -lkissfft-float-openmp -lsndfile -lportaudio

compile:
	$(CC) $(CFLAGS) $(INCLUDE) kissfft-test.cpp $(LDLIBS)

.PHONY: compile
