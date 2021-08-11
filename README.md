# flappy
Flappy Bird clone written in C using OpenGL

## Dependencies
[GLFW](https://www.glfw.org/) (zlib) - Multi-platform library for native OpenGL applications  

## Building
This project is built using POSIX-compatible [make](https://pubs.opengroup.org/onlinepubs/009695399/utilities/make.html).
For unix-like systems, it can be built natively.
For Windows builds, [mingw-w64](http://mingw-w64.org/doku.php) is used to cross-compile the project from a Linux system.

### Linux
```
# debian-based
sudo apt install build-essential libglfw3-dev python3 python3-venv

make
```

### macOS
```
# macOS Big Sur on Apple M1
brew install glfw python3

LIBRARY_PATH=/opt/homebrew/lib/ make
```

### Windows
Windows binaries are built by cross-compiling from Linux:
```
# debian-based
sudo apt install make mingw-w64 python3 python3-venv

make  \
  AR=x86_64-w64-mingw32-ar  \
  CC=x86_64-w64-mingw32-gcc  \
  LDFLAGS=-mwindows  \
  LDLIBS='-Lvendor/lib64/windows/ -lglfw3  \
    -lgdi32 -lkernel32 -lshell32 -luser32'
```
