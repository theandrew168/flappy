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
flappy: src/main.c libflappy.a resources
	@echo "EXE     $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c libflappy.a $(LDLIBS)


# Create the virtualenv for pre/post build scripts
venv:
	python3 -m venv venv/
	./venv/bin/pip install -Uq wheel
	./venv/bin/pip install -Uq -r scripts/requirements.txt


# Declare required resource headers
resource_headers =         \
  src/models/square.h      \
  src/shaders/bg_frag.h    \
  src/shaders/bg_vert.h    \
  src/shaders/bird_frag.h  \
  src/shaders/bird_vert.h  \
  src/shaders/demo_frag.h  \
  src/shaders/demo_vert.h  \
  src/shaders/fade_frag.h  \
  src/shaders/fade_vert.h  \
  src/shaders/pipe_frag.h  \
  src/shaders/pipe_vert.h  \
  src/textures/bg.h        \
  src/textures/bird.h      \
  src/textures/pipe.h

# Express dependencies between header and resource files
src/models/square.h: res/models/square.obj
src/shaders/bg_frag.h: res/shaders/bg.frag
src/shaders/bg_vert.h: res/shaders/bg.vert
src/shaders/bird_frag.h: res/shaders/bird.frag
src/shaders/bird_vert.h: res/shaders/bird.vert
src/shaders/demo_frag.h: res/shaders/demo.frag
src/shaders/demo_vert.h: res/shaders/demo.vert
src/shaders/fade_frag.h: res/shaders/fade.frag
src/shaders/fade_vert.h: res/shaders/fade.vert
src/shaders/pipe_frag.h: res/shaders/pipe.frag
src/shaders/pipe_vert.h: res/shaders/pipe.vert
src/textures/bg.h: res/textures/bg.jpeg
src/textures/bird.h: res/textures/bird.png
src/textures/pipe.h: res/textures/pipe.png

# Convert resource files into headers
.PHONY: resources
resources: venv $(resource_headers)
	mkdir -p src/models/
	mkdir -p src/shaders/
	mkdir -p src/textures/
	./venv/bin/python3 scripts/res2header.py res/models/square.obj src/models/square.h
	./venv/bin/python3 scripts/res2header.py res/shaders/bg.frag src/shaders/bg_frag.h
	./venv/bin/python3 scripts/res2header.py res/shaders/bg.vert src/shaders/bg_vert.h
	./venv/bin/python3 scripts/res2header.py res/shaders/bird.frag src/shaders/bird_frag.h
	./venv/bin/python3 scripts/res2header.py res/shaders/bird.vert src/shaders/bird_vert.h
	./venv/bin/python3 scripts/res2header.py res/shaders/demo.frag src/shaders/demo_frag.h
	./venv/bin/python3 scripts/res2header.py res/shaders/demo.vert src/shaders/demo_vert.h
	./venv/bin/python3 scripts/res2header.py res/shaders/fade.frag src/shaders/fade_frag.h
	./venv/bin/python3 scripts/res2header.py res/shaders/fade.vert src/shaders/fade_vert.h
	./venv/bin/python3 scripts/res2header.py res/shaders/pipe.frag src/shaders/pipe_frag.h
	./venv/bin/python3 scripts/res2header.py res/shaders/pipe.vert src/shaders/pipe_vert.h
	./venv/bin/python3 scripts/res2header.py res/textures/bg.jpeg src/textures/bg.h
	./venv/bin/python3 scripts/res2header.py res/textures/bird.png src/textures/bird.h
	./venv/bin/python3 scripts/res2header.py res/textures/pipe.png src/textures/pipe.h


# Helper target that cleans up build artifacts
.PHONY: clean
clean:
	rm -fr flappy *.exe *.a *.so *.dll src/*.o src/models/ src/shaders/ src/textures/


# Default rule for compiling .c files to .o object files
.SUFFIXES: .c .o
.c.o:
	@echo "CC      $@"
	@$(CC) $(CFLAGS) -c -o $@ $<
