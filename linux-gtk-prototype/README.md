# Notepad++ GTK - Native Linux Port

A fully-featured native Linux port of Notepad++ using GTK3 and Scintilla.

![Version](https://img.shields.io/badge/version-0.1-blue)
![License](https://img.shields.io/badge/license-GPL-green)
![Platform](https://img.shields.io/badge/platform-Linux-orange)

## Overview

This project brings the popular Notepad++ text editor to Linux as a native GTK3 application. It provides a familiar interface with tabbed editing, syntax highlighting for 20+ languages, and comprehensive text manipulation features.

## Features

### ✨ Core Features
- 📑 **Tabbed Interface** - Multiple documents with per-tab state
- 🎨 **Syntax Highlighting** - 20+ languages with automatic detection
- 🔍 **Find & Replace** - With case-sensitive search and replace all
- ⚡ **Fast Performance** - Efficient Scintilla-based editing
- 💾 **Smart Saving** - Auto-detect file changes and modified indicators
- 📊 **Status Bar** - Real-time position, line count, and encoding info
- 💼 **Session Management** - Auto-save/restore tabs on exit/startup ⭐ NEW

### 📝 Editing Features
- Complete undo/redo
- **Multi-cursor editing** (Ctrl+D, Ctrl+Shift+L) ⭐ NEW
- **Column/rectangular selection** (Alt+mouse drag) ⭐ NEW
- Line operations (duplicate, delete, move, transpose, join, split)
- Text transformations (case conversion, trim whitespace)
- **Tab/Space conversion** - Convert tabs to spaces or spaces to tabs ⭐ NEW
- **Sort lines** - Ascending or descending alphabetical sort ⭐ NEW
- Block comment/uncomment (Ctrl+/, Ctrl+Shift+/)
- Smart indentation
- Word wrap toggle
- Line numbering
- EOL format control (Windows/Unix/Mac)
- **EOL conversion** - Convert between CRLF, LF, and CR formats ⭐ NEW
- **Recent files menu** (last 10 files)
- **Auto-save system** - Automatic periodic saving of modified files ⭐ NEW
- Bookmarks with visual indicators (F2, Shift+F2, Ctrl+F2)

### 🔎 Search Features
- Find with case sensitivity (Ctrl+F)
- Find Next/Previous (F3/Shift+F3)
- Find and replace (Ctrl+H)
- Replace all
- **Regular expression support** - Find/Replace with regex patterns ⭐ NEW
- **Incremental search** (Ctrl+I) - Real-time search with highlighting ⭐ NEW
- **Preferences dialog** (Ctrl+,) - Comprehensive settings for editor and display ⭐ NEW
- **Macro recording** (F9/Shift+F9/F10) - Record and playback repetitive tasks ⭐ NEW
- Go to line (Ctrl+G)
- **Find in Files** (Ctrl+Shift+F) - Recursive directory search ⭐ NEW

### 👁️ View Features
- Word wrap
- Zoom in/out/restore
- Show/hide whitespace
- Show/hide EOL markers
- Show/hide line numbers
- **Split view (horizontal/vertical)**
- **Code folding** - Fold/unfold code blocks
- Toggle fold (Ctrl+Shift+F)
- **Full screen mode** (F11) ⭐ NEW
- **Distraction-free mode** (F12) - Minimal UI for focused editing ⭐ NEW

### 🌈 Language Support
Automatic syntax highlighting for:
- C/C++, Java, C#
- Python, Ruby, Perl, PHP
- JavaScript, TypeScript
- Rust, Go
- Shell scripts
- HTML, XML, CSS
- JSON, YAML, TOML
- SQL, Markdown
- And more...

## Installation

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake pkg-config libgtk-3-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake pkg-config gtk3-devel

# Arch Linux
sudo pacman -S base-devel cmake pkg-config gtk3
```

### Building from Source
```bash
# Clone the repository
git clone https://github.com/notepad-plus-plus/notepad-plus-plus
cd notepad-plus-plus/linux-gtk-prototype

# Build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/gtk-proto
```

### Alternative Build (Make)
```bash
make
./gtk-proto
```

## Usage

### Launch
```bash
# Open empty editor
./gtk-proto

# Open file
./gtk-proto myfile.txt

# Open multiple files
./gtk-proto file1.cpp file2.h file3.py
```

### Keyboard Shortcuts

#### File Operations
- `Ctrl+N` - New file
- `Ctrl+O` - Open file
- `Ctrl+S` - Save
- `Ctrl+Shift+S` - Save As
- `Ctrl+W` - Close tab
- `Ctrl+Q` - Quit

#### Edit Operations
- `Ctrl+Z` - Undo
- `Ctrl+Y` - Redo
- `Ctrl+X` - Cut
- `Ctrl+C` - Copy
- `Ctrl+V` - Paste
- `Ctrl+A` - Select All

#### Line Operations
- `Ctrl+D` - Duplicate line
- `Ctrl+L` - Delete line
- `Ctrl+T` - Transpose lines
- `Ctrl+Shift+Up` - Move line up
- `Ctrl+Shift+Down` - Move line down

#### Text Transformations
- `Ctrl+U` - Convert to lowercase
- `Ctrl+Shift+U` - Convert to UPPERCASE
- `Ctrl+Tab` - Indent
- `Ctrl+Shift+Tab` - Unindent

#### Search
- `Ctrl+F` - Find
- `Ctrl+H` - Replace
- `Ctrl+G` - Go to line

#### View
- `Ctrl++` - Zoom in
- `Ctrl+-` - Zoom out
- `Ctrl+0` - Restore default zoom

## Project Structure

```
linux-gtk-prototype/
├── main_gui.cxx          # Main application code
├── npp_gtk.h             # Header definitions
├── CMakeLists.txt        # CMake build configuration
├── Makefile              # Alternative Make build
├── FEATURES.md           # Detailed feature documentation
├── PORTING_PLAN.md       # Development roadmap
└── README.md             # This file

scintilla/                # Scintilla editing component
├── bin/                  # Built libraries
├── include/              # Scintilla headers
└── src/                  # Scintilla source

lexilla/                  # Lexilla lexer library
├── bin/                  # Built lexer libraries
├── include/              # Lexilla headers
├── lexers/               # Language lexers
└── src/                  # Lexilla source
```

## Architecture

### Components

1. **GTK3** - Native Linux GUI framework
   - Window management
   - Menu system
   - Dialogs
   - Toolbar and status bar

2. **Scintilla** - Advanced text editing component
   - Text rendering and editing
   - Syntax styling
   - Line number margins
   - Zoom and view controls

3. **Lexilla** - Syntax highlighting lexers
   - Language detection
   - Token colorization
   - 100+ language lexers

### Design Patterns

- **MVC Pattern**: Separation of UI (GTK) and editing logic (Scintilla)
- **Event-Driven**: GTK signal/callback architecture
- **Tab-Per-Document**: Independent state per editor instance
- **Command Pattern**: Menu/toolbar actions as discrete commands

## Development

### Code Organization

```cpp
// Application state
struct AppState {
    GtkWidget *window;
    GtkWidget *notebook;     // Tab container
    GtkWidget *statusbar;
    GtkAccelGroup *accel_group;
    bool word_wrap;
    bool show_whitespace;
    // ... view settings
};

// Per-tab state
struct TabData {
    GtkWidget *sci;          // Scintilla widget
    string filename;
    bool modified;
};
```

### Adding New Features

1. **Add Command Handler**
```cpp
static void cmd_new_feature(GtkWidget *w, gpointer data) {
    AppState *app = (AppState*)data;
    TabData *td = get_current_tabdata(app);
    // Implementation
}
```

2. **Create Menu Item**
```cpp
GtkWidget *item = gtk_menu_item_new_with_mnemonic("_New Feature");
gtk_widget_add_accelerator(item, "activate", app.accel_group, 
                          GDK_KEY_F, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
```

3. **Connect Signal**
```cpp
g_signal_connect(item, "activate", G_CALLBACK(cmd_new_feature), &app);
```

### Scintilla Messages

Common Scintilla operations:
```cpp
// Get text
scintilla_send_message(sci, SCI_GETTEXT, len, (sptr_t)buffer);

// Set lexer
scintilla_send_message(sci, SCI_SETILEXER, 0, (sptr_t)lexer);

// Line operations
scintilla_send_message(sci, SCI_LINEDUPLICATE, 0, 0);
scintilla_send_message(sci, SCI_LINEDELETE, 0, 0);

// Search
scintilla_send_message(sci, SCI_SEARCHINTARGET, len, (sptr_t)text);
```

## Testing

### Manual Testing
1. Launch application
2. Test file operations (new, open, save, close)
3. Test editing features (undo, redo, copy, paste)
4. Test search functionality
5. Test syntax highlighting with various file types
6. Test view options (zoom, wrap, whitespace)
7. Test keyboard shortcuts

### Automated Testing (TODO)
- Unit tests for command handlers
- Integration tests for file I/O
- UI tests for dialogs
- Performance benchmarks

## Roadmap

### Phase 1: Core Features ✅ COMPLETE
- [x] Basic editor with tabs
- [x] File operations
- [x] Edit menu
- [x] Find/Replace
- [x] Syntax highlighting
- [x] View menu features
- [x] Line operations
- [x] Status bar

### Phase 2: Advanced Editing 🚧 IN PROGRESS
- [ ] Multi-cursor editing
- [ ] Column mode
- [ ] Block comment/uncomment
- [ ] Bookmarks
- [ ] Code folding

### Phase 3: UI Enhancements 📋 PLANNED
- [ ] Preferences dialog
- [ ] Split view
- [ ] Session management
- [ ] Recent files
- [ ] Drag and drop

### Phase 4: Extended Features 📋 PLANNED
- [ ] Find in files
- [ ] Macro recording/playback
- [ ] Plugin architecture
- [ ] Auto-completion
- [ ] Print support

## Contributing

Contributions welcome! Areas of focus:
- Feature implementation
- Bug fixes
- Documentation
- Testing
- Performance optimization

### Contribution Guidelines
1. Fork the repository
2. Create a feature branch
3. Implement changes with clear commits
4. Test thoroughly
5. Submit pull request with description

## Known Issues

1. **GTK Stock Icons Deprecated** - Build warnings (cosmetic only)
2. **Language Menu** - Items created but callbacks not connected
3. **File Close** - No confirmation dialog for unsaved changes yet

## License

GPL v3 - Same as Notepad++ and Scintilla

## Credits

### Original Software
- **Notepad++** by Don Ho - Windows text editor
- **Scintilla** by Neil Hodgson - Editing component
- **Lexilla** by Neil Hodgson - Lexer library

### GTK Port
- Architecture and implementation
- GTK3 integration
- Linux-specific features

## Links

- [Notepad++](https://notepad-plus-plus.org/)
- [Scintilla](https://www.scintilla.org/)
- [Lexilla](https://www.scintilla.org/Lexilla.html)
- [GTK](https://www.gtk.org/)

## Support

For issues and questions:
1. Check existing documentation
2. Search closed issues
3. Open new issue with details

## Changelog

### v0.1 (Current)
- Initial GTK3 port
- Core editing features
- Syntax highlighting for 20+ languages
- Find/Replace functionality
- Line operations
- View controls
- EOL format management

---

**Made with ❤️ for the Linux community**
