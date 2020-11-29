# Request POSIX Make behavior and disable any default suffix-based rules
.POSIX:
.SUFFIXES:


# Declare compiler tools and flags
CC      = cc
CFLAGS  = -std=c99
CFLAGS += -fPIC -g -Og
CFLAGS += -DGLFW_INCLUDE_NONE
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-unused-function
CFLAGS += -Isrc/ -Ivendor/include/
LDFLAGS =
LDLIBS  = -lGL -lglfw


# Declare which targets should be built by default
default: flappy
all: libflappy.a libflappy.so flappy


# Declare library sources
libflappy_sources =    \
  src/opengl.c         \
  src/shader_opengl.c
libflappy_objects = $(libflappy_sources:.c=.o)

# Express dependencies between object and source files
src/opengl.o: src/opengl.c src/opengl.h
src/shader_opengl.o: src/shader_opengl.c src/shader_opengl.h src/shader.h

# Build the static library
libflappy.a: $(libflappy_objects)
	@echo "STATIC  $@"
	@$(AR) rcs $@ $(libflappy_objects)

# Build the shared library
libflappy.so: $(libflappy_objects)
	@echo "SHARED  $@"
	@$(CC) $(LDFLAGS) -shared -o $@ $(libflappy_objects) $(LDLIBS)


# Build the main executable
flappy: src/main.c libflappy.a
	@echo "EXE     $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c libflappy.a $(LDLIBS)


# Helper target that cleans up build artifacts
.PHONY: clean
clean:
	rm -fr flappy *.exe *.a *.so src/*.o


# Default rule for compiling .c files to .o object files
.SUFFIXES: .c .o
.c.o:
	@echo "CC      $@"
	@$(CC) $(CFLAGS) -c -o $@ $<
