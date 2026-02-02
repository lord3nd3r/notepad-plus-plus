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

#### Line Operations
- **Duplicate Line** (Ctrl+D): Duplicate current line
- **Delete Line** (Ctrl+L): Delete current line
- **Cut Line**: Cut entire line to clipboard
- **Copy Line**: Copy entire line to clipboard
- **Move Line Up** (Ctrl+Shift+Up): Move line(s) up
- **Move Line Down** (Ctrl+Shift+Down): Move line(s) down
- **Transpose Lines** (Ctrl+T): Swap current line with next line

#### Text Transformations
- **Convert Case**
  - **UPPERCASE** (Ctrl+Shift+U): Convert selection to uppercase
  - **lowercase** (Ctrl+U): Convert selection to lowercase
- **Indent** (Ctrl+Tab): Indent selection
- **Unindent** (Ctrl+Shift+Tab): Unindent selection
- **Trim Trailing Space**: Remove trailing whitespace from all lines

### Search Menu
- **Find** (Ctrl+F): Search for text with case-sensitive option
- **Replace** (Ctrl+H): Find and replace with Replace/Replace All options
- **Go to Line** (Ctrl+G): Jump to specific line number

### View Menu
- **Word Wrap**: Toggle word wrapping
- **Show Line Numbers**: Toggle line number margin (enabled by default)
- **Show Whitespace**: Display space and tab characters
- **Show EOL**: Display end-of-line markers
- **Zoom In** (Ctrl++): Increase font size
- **Zoom Out** (Ctrl+-): Decrease font size
- **Restore Default Zoom** (Ctrl+0): Reset zoom to default

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
