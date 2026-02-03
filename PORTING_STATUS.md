# Notepad++ Linux GTK Port - Feature Status

## Build Information
- **Language:** C++17
- **GUI Toolkit:** GTK3
- **Editor:** Scintilla
- **Syntax:** Lexilla
- **Build:** CMake + Make wrapper
- **Binary Size:** ~6.5 MB

## Feature Comparison: Windows vs Linux

| Feature Category | Windows | Linux Port | Status |
|-----------------|---------|------------|---------|
| **Core Editing** | Full | Complete | ✅ 100% |
| **Multi-tab Interface** | Full | Complete | ✅ 100% |
| **Undo/Redo** | Full | Complete | ✅ 100% |
| **Multi-cursor Editing** | Full | Complete | ✅ 100% |
| **Column Mode** | Full | Complete | ✅ 100% |
| **Split View** | Full | Complete | ✅ 100% |
| **Syntax Highlighting** | 80+ languages | 20+ languages | 🟡 25% |
| **Search & Replace** | Full | Complete with regex | ✅ 100% |
| **Bookmarks** | Full | Complete | ✅ 100% |
| **Code Folding** | Full | Complete | ✅ 100% |
| **Macros** | Full | Record/Playback | 🟡 60% |
| **Session Management** | Full | Auto-save/restore | ✅ 100% |
| **Auto-completion** | Advanced | Word completion | 🟡 40% |
| **Themes** | Many | 4 themes | 🟡 20% |
| **Plugins** | Extensive | None | ❌ 0% |
| **Print Support** | Full | None | ❌ 0% |
| **Document Map** | Full | None | ❌ 0% |
| **Function List** | Full | None | ❌ 0% |

**Legend:** ✅ Complete | 🟡 Partial | ❌ Not Implemented

## Completed Features (70+ Items)

### Core Editing ✅
- Undo/Redo, Cut/Copy/Paste
- Select All, Select Word
- Multi-cursor editing (Ctrl+D, Ctrl+Shift+L)
- Column/rectangular selection (Alt+drag)
- Multiple tabs with modified indicators
- Real-time status bar

### File Operations ✅
- New, Open, Save, Save As
- Recent Files (last 10)
- Session management (auto-save/restore)
- UTF-8 encoding
- Multi-instance support
- Command-line arguments

### Line Operations ✅
- Duplicate, Delete, Cut/Copy line
- Move line up/down
- Transpose, Join, Split lines

### Text Transformations ✅
- UPPERCASE/lowercase conversion
- Block comment/uncomment
- Increase/decrease indent
- Trim trailing whitespace
- Tab/space conversion
- Sort lines (A-Z, Z-A)

### Search & Replace ✅
- Find/Replace with regex
- Incremental search (Ctrl+I)
- Find Next/Previous
- Find in Files (recursive)
- Go to line

### View Controls ✅
- Word wrap, zoom controls
- Show whitespace/EOL
- Line numbers
- Split view (horizontal/vertical)
- Code folding
- Full screen (F11)
- Distraction-free mode (F12)

### Advanced Features ✅
- 20+ syntax highlighting languages
- Bookmarks with visual indicators
- Auto-save system
- File watching/auto-reload
- 4 color themes
- Macro record/playback
- Preferences dialog

## Planned Features

### High Priority
- [ ] Additional syntax languages (target: 50+)
- [ ] Advanced auto-completion
- [ ] Function list panel
- [ ] Plugin architecture

### Medium Priority
- [ ] Document map
- [ ] Print support
- [ ] More themes
- [ ] Smart highlighting

### Low Priority
- [ ] Backup system
- [ ] Tab context menu
- [ ] Brace matching

## Overall Progress

**Feature Parity:** ~70% of Windows Notepad++  
**Core Functionality:** 100% complete  
**Advanced Features:** 40% complete

---

**Developer:** Kristopher Craig  
**License:** GPL-3.0 (same as Notepad++)
