# Linux GTK Prototype

This folder contains a GTK3 C++ prototype that embeds the Scintilla GTK widget and integrates Lexilla for syntax highlighting.

## Features
- Tabbed document interface (GtkNotebook)
- Toolbar with New/Open/Save actions
- Per-tab file handling with modified indicators
- Status bar showing line, column, and document length
- Line number margin and monospace font
- Syntax highlighting via Lexilla (detects file extension)
- **80+ keyboard shortcuts** (Ctrl+O, Ctrl+S, Ctrl+D, etc.)
- **Multi-cursor editing** (Ctrl+D, Ctrl+Shift+L, Escape)
- **Column/rectangular selection** (Alt+mouse drag)
- **Bookmarks** with visual indicators (F2, Shift+F2, Ctrl+F2)
- **Find/Replace** with Find Next/Previous (F3/Shift+F3)
- **Regular expression support** - Find/Replace with regex patterns
- **Find in Files** - Recursive directory search
- **Recent files menu** (last 10 files)
- **Session management** - Auto-save/restore tabs
- **Split view** - Horizontal and vertical split
- **Line operations** (duplicate, delete, move, transpose, join, split)
- **Text transformations** (case conversion, indent, trim)
- **Block comment/uncomment** (Ctrl+/, Ctrl+Shift+/)
- **View controls** (zoom, wrap, show whitespace/EOL/line numbers)
- **Preferences dialog** (Ctrl+,) - Editor and display settings
- **Macro recording** (F9/Shift+F9/F10) - Automate repetitive tasks
- **Code folding** - Fold/unfold code blocks with visual markers
- **Full screen mode** (F11)

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

# Without arguments (restores last session)
./build/gtk-proto    # for CMake build
# or
./gtk-proto          # for Make build

# With files to open
./build/gtk-proto file1.txt file2.cpp file3.py
```
./gtk-proto          # for Make build
```

If the binary can't find shared libraries at runtime, set `LD_LIBRARY_PATH`:

```bash
LD_LIBRARY_PATH=../scintilla/bin:../lexilla/bin ./build/gtk-proto
```

## Notes
- This is a **full-featured native Linux port** with 1,800+ lines of C++17 code
- Uses the Scintilla GTK widget (`scintilla_object_new()`) and Lexilla for syntax highlighting
- Extension-to-lexer mapping supports 20+ languages: C/C++, Python, JavaScript, Rust, Go, HTML, XML, JSON, Markdown, etc.
- **Multi-cursor editing** works like VS Code (Ctrl+D to add next occurrence)
- **Column mode** enabled with Alt+mouse drag for rectangular selections
- **Split view** allows viewing two panes simultaneously (horizontal or vertical)
- To add more lexers or customize, edit the extension mapping in `main_gui.cxx`
- See [PORTING_STATUS.md](../PORTING_STATUS.md) for complete feature list and roadmap
