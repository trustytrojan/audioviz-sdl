CC = g++
CFLAGS = -Wall -Wextra -Wno-subobject-linkage -std=gnu++23 $(if $(release),-O3,-g)
INCLUDE = -Iinclude -I/usr/include/SDL2
LDLIBS = -lsndfile -lfftw3f -lSDL2 -lSDL2_gfx -lSDL2pp -lportaudio
OBJDIR = obj
BINDIR = bin
SRCDIR = src

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
# List of object files
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Default target
all: clear makedirs $(BINDIR)/a.out

# Linking
$(BINDIR)/a.out: $(OBJS)
	$(CC) $^ $(LDLIBS) -o $@

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Create necessary directories
makedirs:
	mkdir -p $(BINDIR) $(OBJDIR)

# Clean up
clean:
	rm -rf $(BINDIR) $(OBJDIR)

clear:
	clear

.PHONY: all makedirs clean