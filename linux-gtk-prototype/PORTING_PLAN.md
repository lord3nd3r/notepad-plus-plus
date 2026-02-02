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

## Phase 2: Advanced Editing
- [x] Multi-selection/Multi-cursor editing (Ctrl+D, Ctrl+Shift+L)
- [x] Column mode editing (Alt+mouse drag)
- [x] Block comment/uncomment (Ctrl+/, Ctrl+Shift+/)
- [x] Line operations (duplicate, delete, move up/down, split, join)
- [x] Text transformations (uppercase, lowercase, etc.)
- [x] Trim operations
- [x] Tab<->Space conversion ✅ COMPLETE
- [x] Sort lines ✅ COMPLETE
- [x] Auto-completion ✅ COMPLETE
- [ ] Function call tips

## Phase 3: Search & Replace
- [x] Find dialog with case sensitivity
- [x] Replace dialog
- [x] Find Next/Previous (F3/Shift+F3)
- [x] Find in Files ✅ COMPLETE
- [x] Bookmark functionality (F2, Shift+F2, Ctrl+F2)
- [x] Visual bookmark indicators
- [x] Regex support ✅ COMPLETE
- [x] Incremental search ✅ COMPLETE

## Phase 4: View & Display
- [x] Split view (horizontal/vertical) ✅ COMPLETE
- [x] Zoom in/out (Ctrl++/Ctrl+-)
- [x] Restore default zoom (Ctrl+/)
- [x] Word wrap toggle (Ctrl+W)
- [x] Show symbols (whitespace, EOL, etc.)
- [x] Show/hide line numbers
- [x] Code folding ✅ COMPLETE
- [x] Full screen mode (F11) ✅ COMPLETE
- [x] Distraction free mode ✅ COMPLETE

## Phase 5: Language & Encoding
- [ ] Language menu (all lexers)
- [ ] Encoding conversion
- [x] EOL conversion (Windows/Unix/Mac) ✅ COMPLETE
- [ ] BOM handling

## Phase 6: Settings & Preferences
- [ ] Preferences dialog
- [ ] Style configurator
- [ ] Shortcut mapper
- [ ] Theme support (dark mode)

## Phase 7: Advanced Features
- [ ] Macro recording/playback
- [ ] Plugin system architecture
- [x] Session management (auto-save/restore tabs) ✅ COMPLETE
- [x] Backup system ✅ COMPLETE
- [ ] Document map/Function list
- [ ] Clipboard history

## Phase 8: Polish & Optimization
- [ ] Performance optimization
- [ ] Memory management
- [x] File watching/auto-reload ✅ COMPLETE
- [x] Theme support (Dark, Monokai, Solarized) ✅ COMPLETE
- [x] Multi-instance support
- [x] Command line argument handling ✅ COMPLETE
- [ ] Packaging (AppImage, DEB, Flatpak)

## Current Status
**Phase 1**: ✅ Complete
**Phase 2**: ✅ 95% Complete (multi-cursor, column mode, line ops, transformations, sort, auto-completion)
**Phase 3**: ✅ 100% Complete (find/replace, bookmarks, Find in Files, regex, incremental search) ✅
**Phase 4**: ✅ 100% Complete (zoom, wrap, symbols, split view, folding, full screen, distraction-free) ✅
**Phase 5**: ✅ EOL conversion complete
**Phase 7**: ✅ Session management and backup complete
**Phase 8**: ✅ File watching/auto-reload, theme support, and multi-instance support complete

**Current Priority**: Packaging (AppImage, DEB, Flatpak)
**Features Completed**: 95+ keyboard shortcuts, ~3,900 lines of code
**Latest Features**: Tab/space conversion, sort lines, EOL conversion, distraction-free mode, auto-save, auto-completion, multi-instance support, modern UI with icons
