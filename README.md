# flappy
Flappy Bird clone written in C using OpenGL

## Dependencies
[GLFW3](https://www.glfw.org/) (zlib) - Multi-platform library for native OpenGL applications  
[miniaudio](https://miniaud.io/) (public domain) - Single file audio playback and capture library  

## Building
This project is built using POSIX-compatible [make](https://pubs.opengroup.org/onlinepubs/009695399/utilities/make.html).
For unix-like systems, it can be built natively.
For Windows builds, [mingw-w64](http://mingw-w64.org/doku.php) is used to cross-compile the project from a unix-like system.

### Linux
```
# debian-based
sudo apt install gcc make libglfw3-dev python3 python3-venv

make
```

### macOS
```
brew install make python3

make LDLIBS='-Lvendor/lib64/macos/ -lglfw3 -lm  \
  -framework Cocoa -framework OpenGL -framework IOKit'
```

### Windows
Windows binaries are cross-compiled from Linux or macOS.

From Linux:
```
# debian-based
sudo apt install make mingw-w64 python3 python3-venv

make  \
  AR=x86_64-w64-mingw32-ar  \
  CC=x86_64-w64-mingw32-gcc  \
  CFLAGS_EXTRAS=-D__USE_MINGW_ANSI_STDIO  \
  LDFLAGS=-mwindows  \
  LDLIBS='-Lvendor/lib64/windows/ -lgdi32 -lglfw3 -lopengl32'
```

From macOS:
```
brew install make mingw-w64 python3

make  \
  AR=x86_64-w64-mingw32-ar  \
  CC=x86_64-w64-mingw32-gcc  \
  CFLAGS_EXTRAS=-D__USE_MINGW_ANSI_STDIO  \
  LDFLAGS=-mwindows  \
  LDLIBS='-Lvendor/lib64/windows/ -lgdi32 -lglfw3 -lopengl32'
```
