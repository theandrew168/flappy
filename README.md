# flappy
Flappy Bird clone written in C using OpenGL

## Dependencies
[GLFW3](https://www.glfw.org/) (zlib) - Multi-platform library for native OpenGL applications  

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

make LDLIBS='-Lvendor/lib64/macos/ -lglfw3  \
  -framework Cocoa -framework IOKit'
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
  LDFLAGS=-mwindows  \
  LDLIBS='-Lvendor/lib64/windows/ -lglfw3  \
    -lgdi32 -lkernel32 -lshell32 -luser32'
```

From macOS:
```
brew install make mingw-w64 python3

make  \
  AR=x86_64-w64-mingw32-ar  \
  CC=x86_64-w64-mingw32-gcc  \
  LDFLAGS=-mwindows  \
  LDLIBS='-Lvendor/lib64/windows/ -lglfw3  \
    -lgdi32 -lkernel32 -lshell32 -luser32'
```
