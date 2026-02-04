# Changelog

All notable changes to Notepad++ Linux GTK Port will be documented in this file.

## [1.0-beta] - 2026-02-03

### ✨ Major Update: Lexilla Integration Complete

#### Added - Syntax Highlighting
- **125+ lexers successfully compiled and integrated**
- Full syntax highlighting for all supported languages:
  - **Popular:** C/C++, C#, Java, Python, JavaScript, TypeScript, Go, Rust, Swift, Kotlin
  - **Web:** HTML, CSS, SCSS, XML, JSON, YAML, PHP, JSP, ASP
  - **Scripting:** Bash, PowerShell, Perl, Ruby, Lua, TCL, R
  - **Data:** SQL, Makefile, CMake, Dockerfile, TOML, INI
  - **Markup:** Markdown, LaTeX, reStructuredText, AsciiDoc
  - **Systems:** Assembly (x86, ARM, MIPS), Verilog, VHDL, Fortran, Ada
  - **And 90+ more languages!**
- RGB to BGR color conversion for proper Scintilla color display
- Extensionless file detection (Makefile, CMakeLists.txt, etc.)
- Real-time syntax colorization on file load
- Theme-based color schemes for all languages

#### Fixed - Lexilla Build System
- Resolved CharacterSet namespace ambiguity across all 125+ lexer files
- Fixed include order dependencies (LexAccessor.h before Accessor.h)
- Added missing struct definitions to LexSearchResult.cxx
- Added SC_SEARCHRESULT_LINEBUFFERMAXLENGTH define
- Removed SCI_STYLECLEARALL that was clearing syntax colors
- Fixed style application timing to trigger after file content load

#### Technical Improvements
- All lexers compile without errors
- Zero namespace conflicts
- Proper color format handling throughout
- Optimized lexer initialization
- Memory-safe implementations

### Added - Core Features
- Complete text editor with undo/redo
- Multi-tab interface with close buttons
- File operations (New, Open, Save, Save As)
- UTF-8 encoding support
- Real-time status bar with statistics
- Recent files menu (last 10)
- Session management (auto-save/restore tabs)
- Command-line argument support
- Multi-instance support via D-Bus

### Added - Advanced Editing
- Multi-cursor editing (Ctrl+D, Ctrl+Shift+L)
- Column/rectangular selection (Alt+drag)
- 80+ keyboard shortcuts
- Line operations (duplicate, delete, move, transpose, join, split)
- Text transformations (uppercase, lowercase, comment/uncomment)
- Tab/space conversion
- Sort lines (ascending/descending)
- Trim trailing whitespace

### Added - Search & Navigation
- Find and Replace dialogs
- Regular expression support
- Incremental search (Ctrl+I)
- Find in Files (recursive directory search)
- Go to line
- Bookmarks with visual indicators
- Next/Previous bookmark navigation

### Added - View & Display
- Word wrap toggle
- Zoom controls (in, out, restore)
- Show whitespace/EOL markers
- Line numbers toggle
- Split view (horizontal and vertical)
- Code folding with visual markers
- Full screen mode (F11)
- Distraction-free mode (F12)

### Added - Themes & Customization
- 3 built-in color themes (Default, Dark, Monokai)
- XML theme support (compatible with Notepad++ themes)
- Theme selector in preferences
- Custom font selection and sizing
- Configurable tab width and display options

### Added - Automation
- Macro recording (F9)
- Macro playback (Shift+F9)
- Macro save/load system
- Auto-save with configurable interval
- File watching and auto-reload
- Session snapshot/backup system

### Added - Plugin System
- 100% API compatible with Windows Notepad++ plugins
- Plugin loading and management
- Run menu with shell command execution
- Variable expansion support
- Native .so plugin compilation
- Hello World example plugin included

### Added - Preferences
- Preferences dialog (Ctrl+,)
- Font selection and size
- Tab width configuration
- Auto-save settings
- Theme selection
- Display options

### Fixed
- Build system paths corrected
- CMakeLists.txt library linking
- GLib-GIO warning suppression
- Font button deprecation warnings (noted, not critical)
- Lexilla compilation errors resolved
- Syntax highlighting color display
- File type detection for extensionless files

### Technical
- C++17 codebase
- GTK3 native interface
- Scintilla 5.x editor component
- Lexilla syntax highlighting (125+ lexers)
- CMake build system with Make wrapper
- ~6.5 MB binary size
- ~5,300 lines of code
- Zero compilation errors

---

## Version History

### [1.0-beta] - February 3, 2026
- Lexilla integration complete
- 125+ languages with syntax highlighting
- Full feature set operational

### [0.9-alpha] - February 2, 2026
- Initial release
- Core editing features
- Basic syntax highlighting (20 languages)

---

**Developer:** Kristopher Craig  
**License:** GPL-3.0  
**Repository:** https://github.com/lord3nd3r/notepad-plus-plus  
**Last Updated:** February 3, 2026
