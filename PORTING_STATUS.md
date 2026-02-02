# Notepad++ Linux GTK Port - Status Report

## Overview
This is a native Linux port of Notepad++ using GTK3 and Scintilla. The goal is to achieve feature parity with the Windows version while maintaining a native Linux look and feel.

## Build Information
- **Language**: C++17
- **GUI Toolkit**: GTK3 (gtk+-3.0)
- **Editor Component**: Scintilla (GTK widget)
- **Syntax Highlighting**: Lexilla library
- **Build System**: CMake (with alternative Make support)
- **Binary Size**: ~6 MB
- **Source Code**: ~2,505 lines (main_gui.cxx)
- **Latest Commit**: ae5ebf2c9 - Preferences dialog

## Completed Features ✅

### Core Editing
- [x] Undo/Redo (Ctrl+Z, Ctrl+Y)
- [x] Cut/Copy/Paste (Ctrl+X, Ctrl+C, Ctrl+V)
- [x] Delete (Delete key)
- [x] Select All (Ctrl+A)
- [x] Select Word (Ctrl+Alt+W)

### Multi-Cursor Editing ⭐ NEW
- [x] Add Next Occurrence (Ctrl+D) - Select next matching text
- [x] Select All Occurrences (Ctrl+Shift+L) - Select all matches at once
- [x] Multiple cursors typing - Edit all selections simultaneously
- [x] Clear Multiple Selections (Escape)
- [x] Column/Rectangular Selection (Alt+Mouse Drag)

### File Operations
- [x] New File (Ctrl+N)
- [x] Open File (Ctrl+O)
- [x] Save File (Ctrl+S)
- [x] Save As (Ctrl+Shift+S)
- [x] Close All Tabs (Ctrl+Shift+W)
- [x] Recent Files Menu (dynamic, max 10 files)
- [x] Session Management - Auto-save/restore tabs ⭐ NEW
- [x] Save Session - Manual session save
- [x] Load Session - Manual session restore- [x] **Command-line Arguments** - Open files from terminal ⭐ NEW- [x] Quit (Ctrl+Q)

### Line Operations
- [x] Duplicate Line (Ctrl+Alt+D) - Changed from Ctrl+D
- [x] Delete Line (Ctrl+L)
- [x] Cut Line (Ctrl+Shift+X)
- [x] Copy Line (Ctrl+Shift+C)
- [x] Move Line Up (Ctrl+Shift+Up)
- [x] Move Line Down (Ctrl+Shift+Down)
- [x] Transpose Lines (Ctrl+T)
- [x] Join Lines (Ctrl+J)
- [x] Split Lines

### Text Transformations
- [x] UPPERCASE (Ctrl+Shift+U)
- [x] lowercase (Ctrl+U)
- [x] Block Comment (Ctrl+/)
- [x] Block Uncomment (Ctrl+Shift+/)
- [x] Increase Indent (Tab)
- [x] Decrease Indent (Shift+Tab)
- [x] Trim Trailing Space

### Search & Navigation
- [x] Find Dialog (Ctrl+F)
- [x] Replace Dialog (Ctrl+H)
- [x] **Regular Expression Support** - Find/Replace with regex patterns ⭐ NEW
- [x] Find Next (F3)
- [x] Find Previous (Shift+F3)
- [x] Go to Line (Ctrl+G)
- [x] **Find in Files** (Ctrl+Shift+F) - Recursive directory search ⭐ NEW
- [x] File patterns and directory selection
- [x] Results window with line numbers

### Bookmarks
- [x] Toggle Bookmark (F2)
- [x] Next Bookmark (Shift+F2)
- [x] Previous Bookmark (Ctrl+F2)
- [x] Clear All Bookmarks
- [x] Visual bookmark indicators (red circles in margin)

### View Controls
- [x] Word Wrap toggle (Ctrl+W)
- [x] Zoom In (Ctrl++)
- [x] Zoom Out (Ctrl+-)
- [x] Restore Default Zoom (Ctrl+/)
- [x] Show Whitespace toggle
- [x] Show End of Line toggle
- [x] Show Line Numbers toggle
- [x] **Split View** - Horizontal and Vertical
- [x] **Code Folding** - Fold/unfold code blocks
- [x] Toggle fold (Ctrl+Shift+F)
- [x] Fold/Unfold All commands
- [x] **Full Screen Mode** (F11) ⭐ NEW

### Language Support
- [x] 20+ programming languages with syntax highlighting
- [x] Auto-detection by file extension
- Supported languages include:
  - C/C++, C#, Java
  - Python, JavaScript, TypeScript
  - HTML, CSS, XML, JSON
  - PHP, Perl, Ruby, Go
  - Bash, Batch, PowerShell
  - Markdown, LaTeX
  - SQL, Lua
  - and more...

### Encoding & EOL
- [x] EOL Format selection (Windows/Unix/Mac)
- [x] Convert to Windows (CRLF)
- [x] Convert to Unix (LF)
- [x] Convert to Mac (CR)
- [x] UTF-8 support

### Tab Management
- [x] Multiple tabs (notebook interface)
- [x] Modified indicator (*) in tab title
- [x] Next Tab (Ctrl+PageDown)
- [x] Previous Tab (Ctrl+PageUp)
- [x] Close All Tabs

### Status Bar
- [x] Real-time statistics:
  - Total length
  - Line count
  - Current line number
  - Current column number
  - EOL format
  - Encoding
  - Insert/Overwrite mode

### User Interface
- [x] Complete menu system (File, Edit, Search, View, Language, Encoding, Help)
- [x] 80+ keyboard shortcuts
- [x] Toolbar with common actions
- [x] Responsive status bar
- [x] GTK native dialogs

## Planned Features 📋

### Essential Features
- [ ] Split view (horizontal/vertical)
- [ ] Find in Files
- [ ] Session management (save/restore open files)
- [ ] Preferences/Settings dialog
- [ ] Drag and drop file support
- [ ] Auto-save
- [ ] Code folding

### Advanced Features
- [ ] Macro recording and playback
- [ ] Auto-completion
- [ ] Function list
- [ ] Document map
- [ ] Plugin architecture
- [ ] Color schemes/themes
- [ ] Print support

### Quality of Life
- [ ] File change detection
- [ ] Backup files
- [ ] Recent closed files
- [ ] Tab context menu
- [ ] Smart highlighting of selected word
- [ ] Brace matching highlight
- [ ] Auto-indentation
- [ ] Tab to spaces conversion

## Known Issues 🐛
- GTK stock icons deprecated (warnings during compilation)
- No native window icon yet
- Toolbar uses deprecated stock items

## Testing Status
- ✅ Basic editing operations
- ✅ File I/O
- ✅ Syntax highlighting
- ✅ Find/Replace
- ✅ Bookmarks
- ✅ Line operations
- ✅ Text transformations
- ✅ Recent files
- ✅ Join/Split lines
- ✅ Select word
- ✅ Multi-cursor editing (Ctrl+D, Ctrl+Shift+L)
- ✅ Column/rectangular mode (Alt+drag)
- ✅ Split view (horizontal/vertical)

## Performance
- Fast startup time
- Smooth scrolling with large files
- Efficient syntax highlighting
- Low memory footprint
- Multiple cursors with real-time typing

## Compatibility
- Tested on: Linux x86_64
- GTK Version: 3.0+
- C++ Standard: C++17
- Scintilla Version: Latest from Notepad++ source tree

## Repository
- GitHub: https://github.com/lord3nd3r/notepad-plus-plus
- Latest Commit: 849753c6f
- Branch: master

## Build Instructions
```bash
cd linux-gtk-prototype/build
cmake ..
cmake --build .
./gtk-proto
```

## Recent Updates
### Latest (Commit 849753c6f)
- ✨ **Split View**: Horizontal and vertical split panes with GtkPaned
- ✨ Menu items: Split Horizontal, Split Vertical, Unsplit
- 🔧 Added paned widget and notebook2 to AppState
- 🔧 Store split state (is_split, is_horizontal_split)
- ⚡ View two parts of code simultaneously

### Previous (Commit 81afb42a3, 30dec8d91, 0e1f0a2ac)
- ✨ **Multi-Cursor Editing**: Add Next Occurrence (Ctrl+D), Select All Occurrences (Ctrl+Shift+L)
- ✨ **Column Selection**: Alt+mouse drag for rectangular selection mode
- ✨ **Clear Selections**: Escape key to clear multiple cursors
- 🔧 Changed Duplicate Line shortcut to Ctrl+Alt+D (to avoid conflict with Ctrl+D)
- 🔧 Enabled Scintilla multiple selection, additional typing, and virtual space options
- 🔧 Added status bar messages showing selection count
- ⚡ Real-time typing across multiple cursors

### Previous (Commit bb895bd93, f85ba6f5f, 9b9db7cbf)
- ✨ Complete README rewrite for Linux GTK port
- ✨ Added PORTING_STATUS.md comprehensive documentation
- ✨ Recent Files menu with dynamic updates (max 10 files)
- ✨ Join Lines (Ctrl+J) and Split Lines features
- ✨ Select Word (Ctrl+Alt+W)
- 🔧 Fixed Scintilla API usage (TextRangeFull)

### Initial (Commit 3b57f4cef)
- Initial comprehensive implementation with 70+ shortcuts
- All core editing features (undo/redo, cut/copy/paste, etc.)
- Complete menu system (File, Edit, Search, View, Language, Encoding, Help)
- Bookmarks with visual indicators
- Block comment/uncomment
- Find/Replace with Find Next/Previous
- Syntax highlighting for 20+ languages

## Next Steps
1. ✅ ~~Implement multi-cursor editing~~ DONE
2. ✅ ~~Add column (rectangular) selection mode~~ DONE
3. ✅ ~~Implement split view (horizontal/vertical panes)~~ DONE
4. Create preferences dialog
5. Add session management
6. Implement Find in Files
7. Add code folding support
8. Create plugin architecture

## Contributing
This port aims for 1:1 feature parity with Windows Notepad++. Priority is given to features that enhance the core editing experience.

## License
Following Notepad++ GPL-3.0 License
