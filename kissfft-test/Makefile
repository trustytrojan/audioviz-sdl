CC = g++
CFLAGS = -Wall -Wextra -g -std=gnu++23
INCLUDE = -I/usr/local/include/kissfft -I../include
LDLIBS = -lkissfft-float-openmp -lsndfile -lportaudio -lavformat -lavcodec -lavutil -lswresample

compile:
	$(CC) $(CFLAGS) $(INCLUDE) kissfft-test.cpp $(LDLIBS)

.PHONY: compile
