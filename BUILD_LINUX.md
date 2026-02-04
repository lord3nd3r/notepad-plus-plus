# Building Notepad++ for Linux

This guide explains how to build Notepad++ GTK on Linux systems.

## Prerequisites

### Debian/Ubuntu
```bash
sudo apt install build-essential cmake pkg-config libgtk-3-dev git
```

### Fedora/RHEL
```bash
sudo dnf install gcc-c++ cmake pkg-config gtk3-devel git
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake pkg-config gtk3 git
```

## Quick Build

The simplest way to build Notepad++:

```bash
# Clone the repository (if not already done)
git clone https://github.com/notepad-plus-plus/notepad-plus-plus.git
cd notepad-plus-plus

# Build everything (dependencies + application)
make

# Install to /usr/local/bin (optional)
sudo make install
```

The `make` command will automatically:
1. Build Scintilla library (text editing component)
2. Build Lexilla library (syntax highlighting)
3. Build the Notepad++ GTK application

## Manual Build Steps

If you prefer more control over the build process:

### 1. Build Dependencies

```bash
# Build Scintilla
cd scintilla/gtk
make GTK3=1
cd ../..

# Build Lexilla
cd lexilla/src
make
cd ../..
```

### 2. Build Notepad++

Using CMake (recommended):
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

The executables will be in `build/linux-gtk-prototype/`:
- `gtk-proto` - Development version
- `notepad++` - Release version

### 3. Install (Optional)

```bash
# From the build directory
sudo make install
```

This installs `notepad++` to `/usr/local/bin/`.

## Running Without Installing

You can run the application directly from the build directory:

```bash
# From the repository root
./build/linux-gtk-prototype/notepad++

# Or with a file
./build/linux-gtk-prototype/notepad++ myfile.txt
```

## Troubleshooting

### Missing Libraries Error

If you get an error about missing `liblexilla` or `libscintilla`:

```bash
# Clean and rebuild dependencies
cd scintilla/gtk && make clean && make GTK3=1 && cd ../..
cd lexilla/src && make clean && make && cd ../..
```

Then rebuild the main application.

### CMake Can't Find Libraries

If CMake can't find the libraries after building them:

```bash
# Clean CMake cache and reconfigure
rm -rf build
mkdir build
cd build
cmake ..
make
```

### GTK3 Not Found

Ensure GTK3 development packages are installed:

```bash
# Check if pkg-config can find GTK3
pkg-config --modversion gtk+-3.0

# If not found, install the dev packages
sudo apt install libgtk-3-dev  # Debian/Ubuntu
```

## Build Artifacts

After a successful build, you'll have:

- `scintilla/bin/scintilla.a` - Scintilla static library
- `scintilla/bin/libscintilla.so` - Scintilla shared library
- `lexilla/bin/liblexilla.a` - Lexilla static library
- `lexilla/bin/liblexilla.so` - Lexilla shared library
- `build/linux-gtk-prototype/notepad++` - Main executable

**Note:** The `.a` and `.so` files are not tracked in git and must be built on each system.

## Cleaning

```bash
# Clean everything including dependencies
make clean
cd scintilla/gtk && make clean && cd ../..
cd lexilla/src && make clean && cd ../..

# Or just clean the main build
rm -rf build
```

## Development

For development work, you may want to use the `gtk-proto` executable which is built alongside `notepad++`:

```bash
./build/linux-gtk-prototype/gtk-proto
```

Both executables are functionally identical; `notepad++` is the production name.
