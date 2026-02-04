# Repository Information

## Repository Size: ~300 MB

The Notepad++ repository is approximately 300 MB when cloned. This is due to:

### Source Code Breakdown

1. **Scintilla Editor Component** (~50 MB)
   - Complete source code for the Scintilla text editor
   - 200+ source files with extensive editing capabilities
   - Cross-platform support (Windows, Linux, macOS)
   - Located in: `scintilla/`

2. **Lexilla Syntax Highlighting** (~80 MB)
   - 150+ language lexers for syntax highlighting
   - Support for 100+ programming languages and file formats
   - Extensive test files and examples
   - Located in: `lexilla/`

3. **Notepad++ Core** (~100 MB)
   - Main application code
   - Windows-specific implementations
   - Plugins system and examples
   - Icons, themes, and resources
   - Located in: `PowerEditor/`

4. **Linux GTK Port** (~10 MB)
   - Native Linux implementation
   - GTK3 integration
   - Located in: `linux-gtk-prototype/`

5. **Build Files & Documentation** (~60 MB)
   - Visual Studio project files
   - Build configurations
   - Documentation and examples
   - Test files

## What's NOT in the Repository

The following files are **not tracked** in git (they are generated during build):

- `*.o` - Object files
- `*.a` - Static libraries (Scintilla, Lexilla)
- `*.so` - Shared libraries
- `*.exe` - Executables
- `build/` - CMake build directory

These files are listed in `.gitignore` and must be built on each machine.

## First Build Requirements

When you clone the repository on a **new machine**, you need to build the dependencies:

```bash
# The main Makefile does this automatically
make

# Or manually:
cd scintilla/gtk && make GTK3=1
cd ../../lexilla/src && make
```

This generates approximately **15-20 MB** of libraries:
- `scintilla/bin/scintilla.a` (~3 MB)
- `scintilla/bin/libscintilla.so` (~2 MB)
- `lexilla/bin/liblexilla.a` (~6 MB)
- `lexilla/bin/liblexilla.so` (~4 MB)

## Why Not Include Built Libraries?

**Binary files should not be committed to git** because:

1. **Platform-specific**: Libraries built on one Linux distribution may not work on another
2. **Compiler-specific**: Different GCC/Clang versions produce incompatible binaries
3. **Security**: Users should build from source to ensure code integrity
4. **Git bloat**: Binary files can't be efficiently compressed and would bloat the repository
5. **Architecture-specific**: x86_64, ARM64, etc. require different binaries

## Reducing Clone Size

If you want a smaller download, you can use a shallow clone:

```bash
# Clone only the latest commit (much smaller)
git clone --depth 1 https://github.com/notepad-plus-plus/notepad-plus-plus.git

# This reduces download to ~150-200 MB
```

**Trade-off**: Shallow clones don't include git history, so you can't:
- View old commits
- Create branches from historical points
- Perform full git operations

## Comparison to Other Editors

| Editor       | Repository Size | Reason                                    |
|--------------|----------------|-------------------------------------------|
| Notepad++    | ~300 MB        | Complete Scintilla + Lexilla + Windows    |
| VS Code      | ~200 MB        | TypeScript source + Electron              |
| Sublime Text | Closed source  | Binary-only distribution                  |
| Vim          | ~60 MB         | C source only, simpler architecture       |
| Emacs        | ~280 MB        | Complete Lisp environment + extensions    |

The size is justified by the comprehensive feature set and multi-platform support.

## Summary

- **First clone**: ~300 MB download
- **After build**: Additional ~20 MB of local libraries
- **Total disk usage**: ~320-350 MB
- **Build time**: 2-5 minutes on modern hardware
- **Why so large**: Complete editor + syntax + cross-platform support

This is normal for a full-featured text editor with comprehensive language support!
