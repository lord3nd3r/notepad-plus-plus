# Linux GTK Prototype

This folder contains a GTK3 C++ prototype that embeds the Scintilla GTK widget and integrates Lexilla for syntax highlighting.

## Features
- Tabbed document interface (GtkNotebook)
- Toolbar with New/Open/Save actions
- Per-tab file handling with modified indicators
- Status bar showing line, column, and document length
- Line number margin and monospace font
- Syntax highlighting via Lexilla (detects file extension)
- Keyboard shortcuts (Ctrl+O, Ctrl+S)

## Dependencies
- g++ (C++17)
- cmake (>= 3.10) or make
- pkg-config
- GTK 3 development headers (e.g., `libgtk-3-dev` on Debian/Ubuntu)

## Build

### Option 1: CMake (recommended)

```bash
cd linux-gtk-prototype
cmake -S . -B build
cmake --build build -j$(nproc)
```

The binary will be at `build/gtk-proto`.

### Option 2: Make

```bash
cd linux-gtk-prototype
make
```

Both build methods automatically build Scintilla and Lexilla libraries from the repository if not already present.

## Run

```bash
cd linux-gtk-prototype
./build/gtk-proto    # for CMake build
# or
./gtk-proto          # for Make build
```

If the binary can't find shared libraries at runtime, set `LD_LIBRARY_PATH`:

```bash
LD_LIBRARY_PATH=../scintilla/bin:../lexilla/bin ./build/gtk-proto
```

## Notes
- The prototype uses the Scintilla GTK widget (`scintilla_object_new()`) and Lexilla for syntax highlighting.
- Extension-to-lexer mapping supports common languages: C/C++, Python, JavaScript, Rust, Go, HTML, XML, JSON, Markdown, etc.
- To add more lexers or customize, edit the extension mapping in `main_gui.cxx` (around line 100).
