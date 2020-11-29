# flappy
Flappy Bird clone written in C using OpenGL

## Dependencies
[GLFW3](https://www.glfw.org/) (zlib) - Multi-platform library for desktop OpenGL / Vulkan development

## Building
This project is built using POSIX-compatible [make](https://pubs.opengroup.org/onlinepubs/009695399/utilities/make.html).
For unix-like systems, it can be built natively.
For Windows builds, [mingw-w64](http://mingw-w64.org/doku.php) is used to cross-compile the project from a unix-like system.

### Linux
```
# debian-based
sudo apt install gcc make libglfw3-dev

# RHEL-based
sudo dnf install gcc make glfw-devel

# arch-based
sudo pacman -Syu gcc make glfw-x11

make
```

### macOS
```
brew install make
make -f Makefile.macos
```

### Windows
Windows binaries are cross-compiled from Linux or macOS.

From Linux:
```
# debian-based
sudo apt install make mingw-w64

# RHEL-based
sudo dnf install make mingw64-gcc

# arch-based
sudo pacman -Syu make mingw-w64

make -f Makefile.windows
```

From macOS:
```
brew install make mingw-w64
make -f Makefile.windows
```
