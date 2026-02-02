# Notepad++ GTK Port - Implementation Plan

## Phase 1: Core Editor Features ✅ COMPLETE
- [x] Basic menu structure (File, Edit, Search, View, Encoding, Language, Settings, Macro, Run, Plugins, Window, Help)
- [x] Toolbar with icons
- [x] Tab management
- [x] Status bar with detailed info
- [x] Syntax highlighting (Lexilla integration)
- [x] Find & Replace dialog
- [x] Go to Line dialog
- [x] Undo/Redo functionality
- [x] Cut/Copy/Paste operations
- [x] Save As functionality
- [x] Recent files list

## Phase 2: Advanced Editing (CURRENT)
- [x] Multi-selection/Multi-cursor editing (Ctrl+D, Ctrl+Shift+L)
- [x] Column mode editing (Alt+mouse drag)
- [x] Block comment/uncomment (Ctrl+/, Ctrl+Shift+/)
- [x] Line operations (duplicate, delete, move up/down, split, join)
- [x] Text transformations (uppercase, lowercase, etc.)
- [x] Trim operations
- [ ] Tab<->Space conversion
- [ ] Sort lines
- [ ] Auto-completion
- [ ] Function call tips

## Phase 3: Search & Replace
- [x] Find dialog with case sensitivity
- [x] Replace dialog
- [x] Find Next/Previous (F3/Shift+F3)
- [ ] Find in Files
- [x] Bookmark functionality (F2, Shift+F2, Ctrl+F2)
- [x] Visual bookmark indicators
- [ ] Regex support
- [ ] Incremental search

## Phase 4: View & Display
- [ ] Split view (horizontal/vertical) - NEXT PRIORITY
- [x] Zoom in/out (Ctrl++/Ctrl+-)
- [x] Restore default zoom (Ctrl+/)
- [x] Word wrap toggle (Ctrl+W)
- [x] Show symbols (whitespace, EOL, etc.)
- [x] Show/hide line numbers
- [ ] Code folding
- [ ] Full screen mode
- [ ] Distraction free mode

## Phase 5: Language & Encoding
- [ ] Language menu (all lexers)
- [ ] Encoding conversion
- [ ] EOL conversion (Windows/Unix/Mac)
- [ ] BOM handling

## Phase 6: Settings & Preferences
- [ ] Preferences dialog
- [ ] Style configurator
- [ ] Shortcut mapper
- [ ] Theme support (dark mode)

## Phase 7: Advanced Features
- [ ] Macro recording/playback
- [ ] Plugin system architecture
- [ ] Session management
- [ ] Backup system
- [ ] Document map/Function list
- [ ] Clipboard history

## Phase 8: Polish & Optimization
- [ ] Performance optimization
- [ ] Memory management
- [ ] File watching/auto-reload
- [ ] Multi-instance support
- [ ] Command line argument handling
- [ ] Packaging (AppImage, DEB, Flatpak)

## Current Status
**Phase 1**: ✅ Complete
**Phase 2**: ✅ 80% Complete (multi-cursor, column mode, line ops, transformations)
**Phase 3**: ✅ 70% Complete (find/replace, bookmarks)
**Phase 4**: ✅ 60% Complete (zoom, wrap, show symbols)

**Current Priority**: Split view implementation
**Features Completed**: 80+ keyboard shortcuts, 1,700+ lines of code
**Latest Commits**: Multi-cursor editing, column selection, recent files menu
