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
- **Source Code**: ~1,500 lines (main_gui.cxx)

## Completed Features ✅

### Core Editing
- [x] Undo/Redo (Ctrl+Z, Ctrl+Y)
- [x] Cut/Copy/Paste (Ctrl+X, Ctrl+C, Ctrl+V)
- [x] Delete (Delete key)
- [x] Select All (Ctrl+A)
- [x] Select Word (Ctrl+Alt+W)

### File Operations
- [x] New File (Ctrl+N)
- [x] Open File (Ctrl+O)
- [x] Save File (Ctrl+S)
- [x] Save As (Ctrl+Shift+S)
- [x] Close All Tabs (Ctrl+Shift+W)
- [x] Recent Files Menu (dynamic, max 10 files)
- [x] Quit (Ctrl+Q)

### Line Operations
- [x] Duplicate Line (Ctrl+D)
- [x] Delete Line (Ctrl+L)
- [x] Cut Line (Ctrl+Shift+X)
- [x] Copy Line (Ctrl+Shift+C)
- [x] Move Line Up (Ctrl+Shift+Up)
- [x] Move Line Down (Ctrl+Shift+Down)
- [x] Transpose Lines (Ctrl+T)
- [x] Join Lines (Ctrl+J) - NEW
- [x] Split Lines - NEW

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
- [x] Find Next (F3)
- [x] Find Previous (Shift+F3)
- [x] Go to Line (Ctrl+G)

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
- [x] 70+ keyboard shortcuts
- [x] Toolbar with common actions
- [x] Responsive status bar
- [x] GTK native dialogs

## In Progress 🔄

### Multi-Selection Editing
- [ ] Add next occurrence (Ctrl+D)
- [ ] Select all occurrences (Ctrl+Shift+L)
- [ ] Column (rectangular) selection (Alt+Drag)
- [ ] Multiple carets

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
- ⏳ Multi-cursor editing
- ⏳ Column mode
- ⏳ Split view

## Performance
- Fast startup time
- Smooth scrolling with large files
- Efficient syntax highlighting
- Low memory footprint

## Compatibility
- Tested on: Linux x86_64
- GTK Version: 3.0+
- C++ Standard: C++17
- Scintilla Version: Latest from Notepad++ source tree

## Repository
- GitHub: https://github.com/lord3nd3r/notepad-plus-plus
- Latest Commit: 9b9db7cbf
- Branch: master

## Build Instructions
```bash
cd linux-gtk-prototype/build
cmake ..
cmake --build .
./gtk-proto
```

## Recent Updates
### Latest (Commit 9b9db7cbf)
- ✨ Added Recent Files menu with dynamic updates (max 10 files)
- ✨ Implemented Join Lines (Ctrl+J) - joins current line with next
- ✨ Implemented Split Lines - splits lines at edge column
- ✨ Added Select Word (Ctrl+Alt+W) - selects word under cursor
- 🔧 Added `<algorithm>` include for std::find
- 🔧 Fixed forward declaration order

### Previous (Commit 3b57f4cef)
- Initial comprehensive implementation
- All core editing features
- Complete menu system
- Bookmarks with visual indicators
- Block comment/uncomment
- Find/Replace with Find Next/Previous
- 70+ keyboard shortcuts

## Next Steps
1. Implement multi-cursor editing
2. Add column (rectangular) selection mode
3. Implement split view (horizontal/vertical panes)
4. Create preferences dialog
5. Add session management
6. Implement Find in Files
7. Add code folding support
8. Create plugin architecture

## Contributing
This port aims for 1:1 feature parity with Windows Notepad++. Priority is given to features that enhance the core editing experience.

## License
Following Notepad++ GPL-3.0 License
