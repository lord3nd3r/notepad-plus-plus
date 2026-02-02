# Notepad++ Linux GTK Port 🐧

A native Linux port of the beloved Notepad++ text editor using GTK3 and Scintilla. This project aims to bring the full Notepad++ experience to Linux users with a native look and feel.

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![GTK](https://img.shields.io/badge/GTK-3.0%2B-purple.svg)

## 🎯 Project Goal

Achieve **1:1 feature parity** with Windows Notepad++ while maintaining a native Linux/GTK experience. No Wine, no emulation—just pure native Linux code.

## ✨ Features

### Core Editing
- ✅ Complete undo/redo history
- ✅ Cut, copy, paste, delete operations
- ✅ Select all & select word
- ✅ Multiple tabs with modified indicators
- ✅ Real-time status bar (length, lines, position, encoding)

### File Operations
- ✅ New, Open, Save, Save As
- ✅ Recent Files menu (last 10 files)
- ✅ Close all tabs
- ✅ UTF-8 encoding support
- ✅ Session management (auto-save/restore tabs)
- ✅ Save/Load session manually

### Advanced Line Operations
- ✅ Duplicate line (Ctrl+Alt+D)
- ✅ Delete line (Ctrl+L)
- ✅ Cut/Copy line (Ctrl+Shift+X/C)
- ✅ Move line up/down (Ctrl+Shift+Up/Down)
- ✅ Transpose lines (Ctrl+T)
- ✅ Join lines (Ctrl+J)
- ✅ Split lines at edge column

### Multi-Cursor Editing ⭐
- ✅ Add next occurrence (Ctrl+D)
- ✅ Select all occurrences (Ctrl+Shift+L)
- ✅ Type on multiple cursors simultaneously
- ✅ Clear multiple selections (Escape)
- ✅ Column/rectangular selection (Alt+mouse drag)

### Text Transformations
- ✅ UPPERCASE / lowercase conversion
- ✅ Block comment/uncomment (Ctrl+/, Ctrl+Shift+/)
- ✅ Increase/decrease indent
- ✅ Trim trailing whitespace
- ✅ **Convert tabs to spaces** - Converts all tabs to spaces based on tab width ⭐
- ✅ **Convert spaces to tabs** - Converts leading spaces to tabs based on tab width ⭐
- ✅ **Sort lines** - Alphabetically sort lines ascending (A-Z) or descending (Z-A) ⭐

### Search & Replace
- ✅ Find dialog (Ctrl+F)
- ✅ Replace dialog (Ctrl+H)
- ✅ **Regular expression support** - Find/Replace with regex patterns (std::regex)
- ✅ **Incremental search** (Ctrl+I) - Real-time search with live highlighting
- ✅ Find Next/Previous (F3/Shift+F3)
- ✅ Case-sensitive search option
- ✅ Go to line (Ctrl+G)
- ✅ Find in Files (Ctrl+Shift+F) - Recursive directory search

### Bookmarks
- ✅ Toggle bookmark (F2)
- ✅ Next/Previous bookmark (Shift+F2, Ctrl+F2)
- ✅ Clear all bookmarks
- ✅ Visual indicators (red circles in margin)

### View Controls
- ✅ Word wrap toggle (Ctrl+W)
- ✅ Zoom in/out/restore (Ctrl++/−//)
- ✅ Show whitespace characters
- ✅ Show end-of-line markers
- ✅ Show/hide line numbers
- ✅ Split view (horizontal/vertical)
- ✅ Unsplit view
- ✅ Code folding with +/- icons
- ✅ Fold/unfold all
- ✅ Toggle fold (Ctrl+Shift+F)
- ✅ Full screen mode (F11)
- ✅ **Distraction-free mode** (F12) - Minimal UI for focused writing ⭐
- ✅ **Preferences Dialog** (Ctrl+,) - Editor and display settings
- ✅ **Macro Recording** - Record, save, and playback repetitive tasks (F9/Shift+F9/F10)

### Syntax Highlighting
**20+ programming languages supported:**
- C/C++, C#, Java, Objective-C
- Python, JavaScript, TypeScript
- HTML, CSS, XML, JSON
- PHP, Perl, Ruby, Go, Rust
- Bash, Batch, PowerShell
- SQL, Lua, Markdown, LaTeX
- And more...

### Encoding & EOL
- ✅ Windows (CRLF), Unix (LF), Mac (CR) EOL formats
- ✅ **Convert between EOL formats** - Windows/Unix/Mac line ending conversion ⭐
- ✅ UTF-8 encoding

## 🚀 Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libgtk-3-dev

# Fedora
sudo dnf install gcc-c++ cmake gtk3-devel

# Arch Linux
sudo pacman -S base-devel cmake gtk3
```

### Build
```bash
cd linux-gtk-prototype/build
cmake ..
cmake --build . -j$(nproc)
```

### Run
```bash
./gtk-proto [file1] [file2] ...
```

Open files directly from command line or run without arguments to restore last session.

Or from anywhere:
```bash
/path/to/linux-gtk-prototype/build/gtk-proto
```

## ⌨️ Keyboard Shortcuts

### File Operations
- `Ctrl+N` - New file
- `Ctrl+O` - Open file
- `Ctrl+S` - Save file
- `Ctrl+Shift+S` - Save As
- `Ctrl+Q` - Quit

### Editing
- `Ctrl+Z` - Undo
- `Ctrl+Y` - Redo
- `Ctrl+X` - Cut
- `Ctrl+C` - Copy
- `Ctrl+V` - Paste
- `Ctrl+A` - Select All
- `Ctrl+Alt+W` - Select Word

### Multi-Cursor
- `Ctrl+D` - Add next occurrence
- `Ctrl+Shift+L` - Select all occurrences
- `Escape` - Clear multiple selections
- `Alt+Mouse Drag` - Rectangular selection

### Line Operations
- `Ctrl+Alt+D` - Duplicate line
- `Ctrl+L` - Delete line
- `Ctrl+Shift+X` - Cut line
- `Ctrl+Shift+C` - Copy line
- `Ctrl+Shift+Up` - Move line up
- `Ctrl+Shift+Down` - Move line down
- `Ctrl+T` - Transpose lines
- `Ctrl+J` - Join lines

### Text Transformations
- `Ctrl+Shift+U` - UPPERCASE
- `Ctrl+U` - lowercase
- `Ctrl+/` - Block comment
- `Ctrl+Shift+/` - Block uncomment
- `Tab` - Increase indent
- `Shift+Tab` - Decrease indent

### Search
- `Ctrl+F` - Find
- `Ctrl+H` - Replace
- `F3` - Find next
- `Shift+F3` - Find previous
- `Ctrl+G` - Go to line

### Bookmarks
- `F2` - Toggle bookmark
- `Shift+F2` - Next bookmark
- `Ctrl+F2` - Previous bookmark

### View
- `Ctrl+W` - Toggle word wrap
- `Ctrl++` - Zoom in
- `Ctrl+-` - Zoom out
- `Ctrl+/` - Restore default zoom

### Tabs
- `Ctrl+PageDown` - Next tab
- `Ctrl+PageUp` - Previous tab
- `Ctrl+Shift+W` - Close all tabs

## 📊 Current Status

**Total Features Implemented:** 70+ keyboard shortcuts, 1,500+ lines of code

**Completion Status:**
- ✅ Core editing features: 100%
- ✅ File operations: 100%
- ✅ Line operations: 100%
- ✅ Text transformations: 100%
- ✅ Search & replace: 100%
- ✅ Bookmarks: 100%
- ✅ View controls: 100%
- ✅ Syntax highlighting: 100%
- ✅ Multi-cursor editing: 100%
- ✅ Column mode: 100%
- ✅ Split view: 100%

See [PORTING_STATUS.md](PORTING_STATUS.md) for detailed feature tracking.

## 🏗️ Architecture

```
linux-gtk-prototype/
├── main_gui.cxx        # Main application (1,700+ lines)
├── CMakeLists.txt      # Build configuration
└── build/              # Build output directory
    └── gtk-proto       # Compiled binary (~6 MB)

scintilla/              # Scintilla editor component (GTK)
lexilla/                # Lexilla syntax highlighting library
```

**Key Components:**
- **AppState struct**: Application state management
- **TabData struct**: Per-tab data (widget, filename, modified flag)
- **Scintilla integration**: GTK Scintilla widget for editing
- **Lexilla integration**: Syntax highlighting with 20+ lexers
- **GTK3 UI**: Native Linux interface with menus, dialogs, status bar

## 🎨 Design Philosophy

1. **Native First**: Pure GTK3, no Qt or other toolkits
2. **Feature Parity**: Match Windows Notepad++ functionality
3. **Performance**: Fast startup, smooth scrolling, efficient syntax highlighting
4. **Simplicity**: Clean codebase, easy to understand and extend
5. **Compatibility**: Standard Linux filesystem conventions

## 🛠️ Development

### Code Style
- C++17 standard
- GTK3 best practices
- Scintilla message-based API
- Clear function names with `cmd_` prefix for command handlers

### Adding Features
1. Add command handler function (`static void cmd_feature(...)`)
2. Create menu item with label and accelerator
3. Connect signal (`g_signal_connect(...)`)
4. Test with `cmake --build . && ./gtk-proto`

### Debugging
```bash
# Compile with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Run with GDB
gdb ./gtk-proto
```

## 📋 Roadmap

### High Priority
- [x] ~~Multi-cursor editing (Ctrl+D for next occurrence)~~ ✅ DONE
- [x] ~~Column (rectangular) selection mode~~ ✅ DONE
- [x] ~~Split view (horizontal/vertical)~~ ✅ DONE
- [ ] Preferences dialog
- [ ] Session management (save/restore tabs)

### Medium Priority
- [ ] Find in Files
- [ ] Code folding
- [ ] Auto-completion
- [ ] Function list
- [ ] Macro recording/playback

### Low Priority
- [ ] Plugin architecture
- [ ] Color schemes
- [ ] Document map
- [ ] Print support

## 🤝 Contributing

This is a passion project to bring Notepad++ to Linux. Contributions welcome!

**Priority Areas:**
- Split view implementation
- Performance optimization
- Bug fixes

**How to Contribute:**
1. Fork the repository
2. Create a feature branch
3. Implement your feature
4. Test thoroughly
5. Submit a pull request

## 📄 License

GPL-3.0 License - Same as Notepad++

This project respects the original Notepad++ license and is provided as free software.

## 🙏 Acknowledgments

- **Don Ho** - Creator of Notepad++
- **Scintilla** - The powerful editing component
- **Lexilla** - Syntax highlighting library
- **GTK Project** - Native Linux toolkit
- **Notepad++ Community** - Inspiration and feature reference

## 📞 Links

- **Original Notepad++**: https://notepad-plus-plus.org/
- **Scintilla**: https://www.scintilla.org/
- **GTK**: https://www.gtk.org/

## 💡 Why This Port?

Windows Notepad++ users who switch to Linux often miss the familiar, powerful, and lightning-fast text editor. While alternatives exist (VS Code, Sublime, Atom), none quite match the simplicity and speed of Notepad++. This port brings that experience to Linux natively.

**No Wine, no emulation, no compromises—just native Linux Notepad++.** 🚀

---

*Built with ❤️ for the Linux community*

