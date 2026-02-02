# Notepad++ GTK Port - Features

## Overview
This is a native Linux port of Notepad++ using GTK3 and the Scintilla editing component. The application provides a comprehensive text editing experience with syntax highlighting, multiple document interface, and extensive editing features.

## Implemented Features

### File Operations
- **New** (Ctrl+N): Create a new empty document
- **Open** (Ctrl+O): Open existing files with file chooser dialog
- **Save** (Ctrl+S): Save current document
- **Save As** (Ctrl+Shift+S): Save with new filename
- **Close** (Ctrl+W): Close current tab
- **Save Session**: Manually save all open tabs to session file
- **Load Session**: Manually restore tabs from session file
- **Quit** (Ctrl+Q): Exit application

### Edit Menu
#### Basic Operations
- **Undo** (Ctrl+Z): Undo last change
- **Redo** (Ctrl+Y): Redo undone change
- **Cut** (Ctrl+X): Cut selection to clipboard
- **Copy** (Ctrl+C): Copy selection to clipboard
- **Paste** (Ctrl+V): Paste from clipboard
- **Delete**: Delete selection
- **Select All** (Ctrl+A): Select entire document

#### Multi-Cursor Editing ⭐ NEW
- **Add Next Occurrence** (Ctrl+D): Select next matching text and create additional cursor
- **Select All Occurrences** (Ctrl+Shift+L): Select all matching text at once
- **Clear Multiple Selections** (Escape): Remove all but main selection
- **Column Selection** (Alt+Mouse Drag): Rectangular/column selection mode
- **Multiple Cursor Typing**: Edit all selections simultaneously

#### Line Operations
- **Duplicate Line** (Ctrl+Alt+D): Duplicate current line
- **Delete Line** (Ctrl+L): Delete current line
- **Cut Line** (Ctrl+Shift+X): Cut entire line to clipboard
- **Copy Line** (Ctrl+Shift+C): Copy entire line to clipboard
- **Move Line Up** (Ctrl+Shift+Up): Move line(s) up
- **Move Line Down** (Ctrl+Shift+Down): Move line(s) down
- **Transpose Lines** (Ctrl+T): Swap current line with next line
- **Join Lines** (Ctrl+J): Join current line with next
- **Split Lines**: Split long lines at edge column
- **Select Word** (Ctrl+Alt+W): Select word under cursor

#### Text Transformations
- **Convert Case**
  - **UPPERCASE** (Ctrl+Shift+U): Convert selection to uppercase
  - **lowercase** (Ctrl+U): Convert selection to lowercase
- **Indent** (Ctrl+Tab): Indent selection
- **Unindent** (Ctrl+Shift+Tab): Unindent selection
- **Trim Trailing Space**: Remove trailing whitespace from all lines

#### Block Operations
- **Block Comment** (Ctrl+/): Comment selected lines
- **Block Uncomment** (Ctrl+Shift+/): Uncomment selected lines

### Search Menu
- **Find** (Ctrl+F): Search for text with case-sensitive option
- **Find Next** (F3): Find next occurrence
- **Find Previous** (Shift+F3): Find previous occurrence
- **Replace** (Ctrl+H): Find and replace with Replace/Replace All options
- **Go to Line** (Ctrl+G): Jump to specific line number

### Bookmarks
- **Toggle Bookmark** (F2): Add/remove bookmark on current line
- **Next Bookmark** (Shift+F2): Jump to next bookmark
- **Previous Bookmark** (Ctrl+F2): Jump to previous bookmark
- **Clear All Bookmarks**: Remove all bookmarks
- **Visual Indicators**: Red circles in margin for bookmarked lines

### File Management
- **Recent Files Menu**: Quick access to last 10 opened files
- **Modified Indicator**: Asterisk (*) in tab title for unsaved changes
- **Multiple Tabs**: Notebook interface with tab navigation (Ctrl+PageDown/Up)
- **Session Management** ⭐ NEW:
  - Auto-save all open tabs on exit
  - Auto-restore tabs on startup
  - Session file: `~/.config/notepad-plus-plus-gtk/session.txt`
  - Manual save/load via File menu

### View Menu
- **Word Wrap**: Toggle word wrapping
- **Show Line Numbers**: Toggle line number margin (enabled by default)
- **Show Whitespace**: Display space and tab characters
- **Show EOL**: Display end-of-line markers
- **Zoom In** (Ctrl++): Increase font size
- **Zoom Out** (Ctrl+-): Decrease font size
- **Restore Default Zoom** (Ctrl+0): Reset zoom to default
- **Split Horizontal** ⭐ NEW: Split view horizontally (top/bottom panes)
- **Split Vertical** ⭐ NEW: Split view vertically (left/right panes)
- **Unsplit** ⭐ NEW: Return to single pane view

### Language Menu
Manual syntax highlighting selection for:
- Plain Text
- C
- C++
- Java
- JavaScript
- Python
- Ruby
- Perl
- PHP
- Shell
- Go
- Rust
- SQL
- HTML
- XML
- CSS
- JSON
- Markdown
- YAML

### Encoding Menu
#### EOL Format
- **Windows (CRLF)**: Set line endings to Windows format
- **Unix (LF)**: Set line endings to Unix format (default)
- **Mac (CR)**: Set line endings to classic Mac format
- **Convert EOL**: Convert existing line endings to current format

## Automatic Features

### Syntax Highlighting
Automatically detects file type based on extension and applies appropriate syntax highlighting:
- **C/C++**: .c, .cpp, .cxx, .cc, .h, .hpp, .hxx
- **Python**: .py
- **JavaScript**: .js
- **Rust**: .rs
- **Go**: .go
- **Ruby**: .rb
- **PHP**: .php
- **Perl**: .pl, .pm
- **Shell**: .sh, .bash
- **SQL**: .sql
- **HTML**: .html, .htm
- **CSS**: .css
- **XML**: .xml
- **JSON**: .json
- **Markdown**: .md
- **YAML**: .yaml, .yml
- **TOML**: .toml
- **Java**: .java
- **TypeScript**: .ts
- **C#**: .cs

### Status Bar
Real-time information display:
- Document length (characters)
- Line count
- Current line and column position
- Character encoding (UTF-8)
- Insert/Overwrite mode

### Modified Indicator
- Asterisk (*) in tab title when document has unsaved changes
- Removed when document is saved

## User Interface

### Tabbed Interface
- Multiple documents open simultaneously
- Tab switching
- Per-tab state management
- Modified indicator in tab titles

### Toolbar
Quick access buttons:
- New document
- Open file
- Save file

### Menu Bar
Complete menu system with keyboard shortcuts and mnemonics

## Technical Details

### Components
- **GTK3**: GUI framework
- **Scintilla**: Advanced text editing component
- **Lexilla**: Syntax highlighting lexer library
- **C++17**: Implementation language

### Build System
- CMake (primary)
- Make (alternative)

### Performance
- Efficient syntax highlighting
- Fast file loading and saving
- Responsive UI

## Future Enhancements (Planned)
- Multi-cursor editing
- Column mode editing
- Block comment/uncomment
- Split view
- Macro recording and playback
- Plugin architecture
- Find in files
- Bookmarks
- Code folding
- Auto-completion
- Preferences dialog
- Session management
- Recent files list
- Drag and drop
- Print support

## Comparison with Windows Notepad++

### Implemented (Feature Parity)
- ✅ Tabbed interface
- ✅ Syntax highlighting (20+ languages)
- ✅ Find/Replace
- ✅ Line operations
- ✅ Text transformations
- ✅ Zoom controls
- ✅ EOL format control
- ✅ Status bar with detailed info
- ✅ Keyboard shortcuts
- ✅ Modified indicator

### Not Yet Implemented
- ❌ Multi-cursor editing
- ❌ Column mode
- ❌ Plugin system
- ❌ Macro recording
- ❌ Find in files
- ❌ Code folding
- ❌ Auto-completion
- ❌ Split view
- ❌ Session management

## Usage Notes

### Building
```bash
cd linux-gtk-prototype
cmake -B build
cmake --build build -j$(nproc)
```

### Running
```bash
./build/gtk-proto [filename]
```

### Dependencies
- GTK+ 3.0
- pkg-config
- C++17 compiler

## Known Issues
- GTK stock icons deprecated (cosmetic warnings during build)
- Language menu items not yet connected to callbacks (menu created but inactive)

## Credits
- Based on Notepad++ (Windows)
- Uses Scintilla editing component
- Uses Lexilla lexer library
- GTK3 for native Linux integration
