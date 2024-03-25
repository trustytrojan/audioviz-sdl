CC = g++
CFLAGS = -Wall -Wextra
INCLUDE = -I/usr/include/SDL2
LDLIBS = -lSDL2 -lSDL2_gfx -lSDL2pp
OBJDIR = obj
BINDIR = bin
SRCDIR = src

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.cpp)
# List of object files
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Default target
all: makedirs $(BINDIR)/a.out

# Compile, link, and run
run: all
	$(BINDIR)/a.out

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

.PHONY: all makedirs clean