# Request POSIX Make behavior and disable any default suffix-based rules
.POSIX:
.SUFFIXES:

# Build requirements references:
# https://www.glfw.org/docs/latest/build_guide.html

# CFLAGS breakout by category
CFLAGS_VERSION = -std=c99
CFLAGS_OPTIMIZATIONS = -g -Og
CFLAGS_WARNINGS = -Wall -Wextra -Wpedantic
CFLAGS_DEFINITIONS = -DGLFW_INCLUDE_NONE
CFLAGS_INCLUDE_DIRS = -Ires/ -Isrc/ -Ivendor/include/
CFLAGS_EXTRAS = -pthread

# Declare compiler tools and flags
AR      = ar
CC      = cc
CFLAGS  = -fPIC
CFLAGS += $(CFLAGS_VERSION)
CFLAGS += $(CFLAGS_OPTIMIZATIONS)
CFLAGS += $(CFLAGS_WARNINGS)
CFLAGS += $(CFLAGS_DEFINITIONS)
CFLAGS += $(CFLAGS_INCLUDE_DIRS)
CFLAGS += $(CFLAGS_EXTRAS)
LDFLAGS = -pthread
LDLIBS  = -ldl -lGL -lglfw -lm

# Declare which targets should be built by default
default: flappy

# Declare library sources
libflappy_sources =  \
  src/game.c         \
  src/model.c        \
  src/opengl.c       \
  src/physics.c      \
  src/shader.c       \
  src/texture.c
libflappy_objects = $(libflappy_sources:.c=.o)

# Express dependencies between object and source files
src/game.o: src/game.c src/game.h src/model.h src/opengl.h src/physics.h src/shader.h src/texture.h
src/model.o: src/model.c src/model.h src/opengl.h
src/opengl.o: src/opengl.c src/opengl.h
src/physics.o: src/physics.c src/physics.h
src/shader.o: src/shader.c src/shader.h src/opengl.h
src/texture.o: src/texture.c src/texture.h src/opengl.h

# Build the static library
libflappy.a: $(libflappy_objects)
	@echo "STATIC  $@"
	@$(AR) rcs $@ $(libflappy_objects)

# Double suffix rule for compiling .c files to .o object files
.SUFFIXES: .c .o
.c.o:
	@echo "CC      $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

# Declare required resource headers
resource_headers =           \
  res/models/sprite.h        \
  res/shaders/sprite_frag.h  \
  res/shaders/sprite_vert.h  \
  res/textures/bg.h          \
  res/textures/bird.h        \
  res/textures/pipe_bot.h    \
  res/textures/pipe_top.h

# Express dependencies between header and resource files
res/models/sprite.h: res/models/sprite.obj
res/shaders/sprite_frag.h: res/shaders/sprite_frag.glsl
res/shaders/sprite_vert.h: res/shaders/sprite_vert.glsl
res/textures/bg.h: res/textures/bg.jpg
res/textures/bird.h: res/textures/bird.png
res/textures/pipe_bot.h: res/textures/pipe_bot.png
res/textures/pipe_top.h: res/textures/pipe_top.png

# Resource conversion requires some Python packages
$(resource_headers): venv

# Compile and link the main executable
flappy: src/main.c libflappy.a $(resource_headers)
	@echo "EXE     $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c libflappy.a $(LDLIBS)

# Create the virtualenv for pre/post build scripts
venv:
	@echo "VENV    venv/"
	@python3 -m venv venv/
	@./venv/bin/pip install -Uq wheel
	@echo "DEPS    scripts/requirements.txt"
	@./venv/bin/pip install -Uq -r scripts/requirements.txt

# Double suffix rules for convertion resource files to header files
.SUFFIXES: .obj .h
.obj.h:
	@echo "MODEL   $@"
	@./venv/bin/python3 scripts/res2header.py $< $@

.SUFFIXES: .glsl .h
.glsl.h:
	@echo "SHADER  $@"
	@./venv/bin/python3 scripts/res2header.py $< $@

.SUFFIXES: .jpg .h
.jpg.h:
	@echo "TEXTURE $@"
	@./venv/bin/python3 scripts/res2header.py $< $@

.SUFFIXES: .png .h
.png.h:
	@echo "TEXTURE $@"
	@./venv/bin/python3 scripts/res2header.py $< $@

# Helper target that cleans up build artifacts
.PHONY: clean
clean:
	rm -fr flappy *.exe *.a *.so *.dll src/*.o res/models/*.h res/shaders/*.h res/textures/*.h
