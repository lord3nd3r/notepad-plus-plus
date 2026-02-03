# Notepad++ Linux Port

A native Linux port of Notepad++ using GTK3 and Scintilla. No Wine, no emulation—just pure native Linux code.

![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Version](https://img.shields.io/badge/version-1.0--beta-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![GTK](https://img.shields.io/badge/GTK-3.0+-purple.svg)


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
- Multi-instance support
- Auto-save with configurable interval
- File watching and auto-reload

### Syntax Highlighting
**20+ languages supported:** C/C++, C#, Java, Python, JavaScript, TypeScript, HTML, CSS, XML, JSON, PHP, Perl, Ruby, Go, Rust, Bash, PowerShell, SQL, Lua, Markdown, LaTeX, and more.

### Customization
- Preferences dialog (Ctrl+,)
- Multiple color themes (Default, Dark, Monokai, Solarized)
- Configurable fonts and display settings
- Macro recording and playback (F9/Shift+F9/F10)

### Plugin Support
- **Plugin System**: 100% API compatible with Windows Notepad++ plugins.
  - Supports `setInfo`, `getName`, `getFuncsArray`, `beNotified`, and host messages.
  - "Plugins Admin" dialog for managing loaded plugins.
- **Run Menu**:
  - Execute shell commands.
  - Variable expansion: `$(FULL_CURRENT_PATH)`, `$(FILE_NAME)`, `$(CURRENT_DIRECTORY)`.
  - integrated Web Search and Browser launching.
- **Themes**:
  - Native XML Theme support (compatible with Notepad++ `stylers.xml`).
  - Drop themes into `~/.config/notepad-plus-plus-gtk/themes/`.
- **Session Snapshot**:
  - Automatic backup of unsaved files every 7 seconds.
  - Crash protection saves to `~/.config/notepad-plus-plus-gtk/backup/`.
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
| **Syntax Highlighting** | ✅ 80+ languages | ✅ 20+ languages |
| **Search & Replace** | ✅ Full | ✅ Complete |
| **Multi-cursor Editing** | ✅ Full | ✅ Complete |
| **Column Mode** | ✅ Full | ✅ Complete |
| **Split View** | ✅ Full | ✅ Complete |
| **Macros** | ✅ Full | ✅ Basic (record/playback) |
| **Plugins** | ✅ Extensive | ✅ 100% Backwards Compatible |
| **Themes** | ✅ Many | ✅ 4 themes |
| **Auto-completion** | ✅ Advanced | ✅ Word completion |
| **Session Management** | ✅ Full | ✅ Complete |
| **Code Folding** | ✅ Full | ✅ Complete |
| **Print Support** | ✅ Full | ❌ Not yet |
| **Document Map** | ✅ Full | ❌ Not yet |
| **Function List** | ✅ Full | ❌ Planned |
| **Desktop Integration** | ✅ Full | ✅ Complete (.desktop, Icon) |
| **Packaging** | ✅ MSI/EXE | ✅ AppImage, DEB, RPM |

**Overall Completion:** ~80% feature parity with Windows version

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
├── scintilla/              # Scintilla editor component
├── lexilla/                # Syntax highlighting library
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

## Contributing

Contributions are welcome! Priority areas:
- Additional syntax highlighting languages
- Performance optimization
- Bug fixes and testing

## Links

- **Original Notepad++:** https://notepad-plus-plus.org/
- **Scintilla:** https://www.scintilla.org/
- **GTK:** https://www.gtk.org/

## License

GPL-3.0 License - Same as Notepad++

This project respects the original Notepad++ license and is provided as free software.

---

**Developer:** Kristopher Craig  
*Built with ❤️ for the Linux community*
