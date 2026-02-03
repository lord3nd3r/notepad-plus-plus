# Changelog

All notable changes to Notepad++ Linux GTK Port will be documented in this file.

## [1.0-beta] - 2026-02-02

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

### Added - Syntax & Themes
- Syntax highlighting for 20+ languages
  - C/C++, C#, Java, Python, JavaScript, TypeScript
  - HTML, CSS, XML, JSON, PHP, Perl, Ruby
  - Go, Rust, Bash, SQL, Lua, Markdown, LaTeX
- 4 color themes (Default, Dark, Monokai, Solarized Dark)
- Theme selector in preferences

### Added - Automation
- Macro recording (F9)
- Macro playback (Shift+F9)
- Macro save/load system
- Auto-save with configurable interval
- File watching and auto-reload

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

### Technical
- C++17 codebase
- GTK3 native interface
- Scintilla editor component
- Lexilla syntax highlighting
- CMake build system with Make wrapper
- ~6.5 MB binary size
- ~3,800 lines of code

---

**Developer:** Kristopher Craig  
**License:** GPL-3.0  
**Repository:** https://github.com/lord3nd3r/notepad-plus-plus
