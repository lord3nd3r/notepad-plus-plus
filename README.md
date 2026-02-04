# Notepad++ Linux Port

A native Linux port of Notepad++ using GTK3 and Scintilla. No Wine, no emulation—just pure native Linux code.

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Version](https://img.shields.io/badge/version-1.0--beta-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![GTK](https://img.shields.io/badge/GTK-3.0+-purple.svg)
![Syntax](https://img.shields.io/badge/lexers-125+-brightgreen.svg)


## ✨ Latest Updates (February 2026)

### ✅ Lexilla Integration Complete!
- **All 125+ lexers successfully compiled and integrated**
- **Syntax highlighting now fully functional for all supported languages**
- Fixed CharacterSet namespace conflicts across entire codebase
- Proper RGB to BGR color conversion for Scintilla
- Auto-detection for extensionless files (Makefile, etc.)

## Quick Start

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libgtk-3-dev

# Fedora
sudo dnf install gcc-c++ cmake gtk3-devel

# Arch Linux
sudo pacman -S base-devel cmake gtk3
```

### Build & Install
```bash
git clone https://github.com/lord3nd3r/notepad-plus-plus.git
cd notepad-plus-plus
make
sudo make install
```

### Run
```bash
notepad++
```

## Features

### Core Editing
- Multi-tab interface with close buttons
- Complete undo/redo history
- Cut, copy, paste operations
- Multi-cursor editing (Ctrl+D)
- Column/rectangular selection
- Word wrap, line numbers, zoom
- Split view (horizontal/vertical)

### Advanced Operations
- Line operations (duplicate, delete, move, transpose, join)
- Text transformations (uppercase/lowercase, trim whitespace)
- Tab/space conversion
- Sort lines (ascending/descending)
- Block comment/uncomment

### Search & Replace
- Find and Replace with regex support
- Incremental search (Ctrl+I)
- Find in Files (recursive directory search)
- Go to line
- Bookmarks with navigation

### File Management
- UTF-8 encoding support
- EOL conversion (Windows/Unix/Mac)
- Session management (auto-save/restore tabs)
- Recent files menu
- Multi-instance support via D-Bus
- Auto-save with configurable interval
- File watching and auto-reload

### Syntax Highlighting ✅
**125+ languages fully supported:**

**Popular Languages:** C, C++, C#, Java, Python, JavaScript, TypeScript, Go, Rust, Swift, Kotlin, Scala

**Web Development:** HTML, CSS, SCSS, SASS, XML, JSON, YAML, TOML, PHP, JSP, ASP

**Scripting:** Bash, PowerShell, Perl, Ruby, Lua, TCL, Python, R

**Data & Config:** SQL, Makefile, CMake, Dockerfile, YAML, TOML, INI, Properties

**Markup & Docs:** Markdown, LaTeX, reStructuredText, AsciiDoc, Txt2tags

**Systems:** Assembly (x86, ARM, MIPS), Verilog, VHDL, Fortran, Ada

**And 90+ more!** Including: Haskell, Erlang, Lisp, Scheme, Prolog, D, Nim, Zig, Julia, and many others.

### Customization
- Preferences dialog (Ctrl+,)
- Multiple color themes (Default, Dark, Monokai)
- XML theme support (compatible with Notepad++ themes)
- Configurable fonts and display settings
- Macro recording and playback (F9/Shift+F9/F10)

### Plugin Support
- **Plugin System**: 100% API compatible with Windows Notepad++ plugins
  - Supports `setInfo`, `getName`, `getFuncsArray`, `beNotified`, and host messages
  - "Plugins Admin" dialog for managing loaded plugins
- **Run Menu**:
  - Execute shell commands
  - Variable expansion: `$(FULL_CURRENT_PATH)`, `$(FILE_NAME)`, `$(CURRENT_DIRECTORY)`
  - Integrated Web Search and Browser launching
- **Themes**:
  - Native XML Theme support (compatible with Notepad++ `stylers.xml`)
  - Drop themes into `~/.config/notepad-plus-plus-gtk/themes/`
- **Session Snapshot**:
  - Automatic backup of unsaved files every 7 seconds
  - Crash protection saves to `~/.config/notepad-plus-plus-gtk/backup/`
- **Native Performance:** Plugins compile to native Linux shared libraries (`.so`)
- **Hello World Example:** Included in `plugins/HelloWorld/`
- **Easy Porting:** Most Windows plugins require NO code changes

### Packaging & Distribution
- **AppImage:** Universal portable executable
- **DEB:** Native package for Debian/Ubuntu/Mint
- **RPM:** Native package for Fedora/RHEL/CentOS
- **Desktop Integration:** Application menu entry, icons, and file associations

### Modes
- Full screen mode (F11)
- Distraction-free mode (F12)
- Code folding with visual markers

## Status: Windows vs Linux

| Feature Category | Windows Notepad++ | Linux Port |
|-----------------|-------------------|------------|
| **Core Editing** | ✅ Full | ✅ Complete |
| **Multi-tab Interface** | ✅ Full | ✅ Complete |
| **Syntax Highlighting** | ✅ 125+ languages | ✅ 125+ languages |
| **Search & Replace** | ✅ Full | ✅ Complete |
| **Multi-cursor Editing** | ✅ Full | ✅ Complete |
| **Column Mode** | ✅ Full | ✅ Complete |
| **Split View** | ✅ Full | ✅ Complete |
| **Macros** | ✅ Full | ✅ Basic (record/playback) |
| **Plugins** | ✅ Extensive | ✅ 100% Backwards Compatible |
| **Themes** | ✅ Many | ✅ 3 built-in + XML support |
| **Auto-completion** | ✅ Advanced | ✅ Word completion |
| **Session Management** | ✅ Full | ✅ Complete |
| **Code Folding** | ✅ Full | ✅ Complete |
| **Print Support** | ✅ Full | ❌ Not yet |
| **Document Map** | ✅ Full | ❌ Not yet |
| **Function List** | ✅ Full | ❌ Planned |
| **Desktop Integration** | ✅ Full | ✅ Complete (.desktop, Icon) |
| **Packaging** | ✅ MSI/EXE | ✅ AppImage, DEB, RPM |

**Overall Completion:** ~75% feature parity with Windows version

## Essential Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| New file | Ctrl+N |
| Open file | Ctrl+O |
| Save | Ctrl+S |
| Find | Ctrl+F |
| Replace | Ctrl+H |
| Go to line | Ctrl+G |
| Undo/Redo | Ctrl+Z / Ctrl+Y |
| Multi-cursor (next) | Ctrl+D |
| Comment block | Ctrl+/ |
| Duplicate line | Ctrl+Alt+D |
| Delete line | Ctrl+L |
| Word wrap | Ctrl+W |
| Full screen | F11 |
| Distraction-free | F12 |
| Preferences | Ctrl+, |

See BUILD.md for complete keyboard shortcut reference.

## Project Structure

```
notepad-plus-plus/
├── linux-gtk-prototype/    # Linux GTK port
│   ├── main_gui.cxx        # Main application
│   ├── PluginInterface.h   # Plugin API
│   └── CMakeLists.txt      # Build configuration
├── scintilla/              # Scintilla editor component (v5.x)
├── lexilla/                # Syntax highlighting library (125+ lexers)
├── plugins/                # Plugin SDK and examples
├── packages/               # Packaging scripts (AppImage/DEB/RPM)
├── Makefile                # Build wrapper
└── README.md               # This file
```

## Make Targets

```bash
make          # Build the project
make install  # Install to /usr/local/bin (requires sudo)
make clean    # Remove build artifacts
make rebuild  # Clean and rebuild
make run      # Build and run
make help     # Show all targets
```

## Technical Details

### Build System
- **Language:** C++17
- **GUI Toolkit:** GTK3
- **Editor Component:** Scintilla 5.x
- **Lexer Library:** Lexilla (all 125+ lexers functional)
- **Build Tools:** CMake + Make wrapper
- **Binary Size:** ~6.5 MB
- **Dependencies:** GTK3, Pango, Cairo

### Recent Fixes
- ✅ Resolved CharacterSet namespace conflicts across 125+ lexer files
- ✅ Fixed include order dependencies (LexAccessor.h before Accessor.h)
- ✅ Added missing struct definitions to LexSearchResult.cxx
- ✅ Implemented RGB to BGR color conversion for Scintilla
- ✅ Fixed style application timing for proper syntax highlighting
- ✅ Added support for extensionless files (Makefile, CMakeLists.txt, etc.)

## Contributing

Contributions are welcome! Priority areas:
- Performance optimization
- Additional themes
- Plugin development
- Bug fixes and testing
- Documentation improvements

See CONTRIBUTING.md for guidelines.

## Links

- **Original Notepad++:** https://notepad-plus-plus.org/
- **Scintilla:** https://www.scintilla.org/
- **Lexilla:** https://www.scintilla.org/Lexilla.html
- **GTK:** https://www.gtk.org/

## License

GPL-3.0 License - Same as Notepad++

This project respects the original Notepad++ license and is provided as free software.

---

**Developer:** Kristopher Craig  
**Last Updated:** February 3, 2026  
*Built with ❤️ for the Linux community*
