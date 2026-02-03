# Notepad++ Linux GTK Port - Version 1.0 (BETA)

**Release Date:** February 2, 2026  
**Developer:** Kristopher Craig  
**License:** GPL-3.0

## Release Highlights

This is the **first beta release** of the native Linux GTK port of Notepad++. After extensive development, we've achieved approximately **70% feature parity** with the Windows version, with all core editing features fully functional.

### What's Included

✅ **Complete Core Editor** - All essential text editing features  
✅ **Multi-cursor Editing** - Productivity-boosting multi-cursor support  
✅ **Split View** - Horizontal and vertical panes  
✅ **Code Folding** - Collapsible code blocks  
✅ **Syntax Highlighting** - 20+ programming languages  
✅ **Session Management** - Auto-save and restore  
✅ **Theme Support** - 4 color themes included  
✅ **Macro System** - Record and playback macros  
✅ **80+ Keyboard Shortcuts** - Windows-compatible shortcuts  

### Installation

```bash
git clone https://github.com/lord3nd3r/notepad-plus-plus.git
cd notepad-plus-plus
make
sudo make install
notepad++
```

### System Requirements

- **OS:** Linux with GTK 3.0+
- **Tested on:** Ubuntu 20.04+, Debian 10+, Fedora 33+, Arch Linux
- **Build:** GCC/Clang with C++17, CMake >= 3.10

### Known Issues

- Some GLib warnings in older systems (harmless, suppressed in v1.0)
- Plugin system not yet implemented
- Print support not available
- Limited to 20 syntax highlighting languages (vs 80+ in Windows)

### Feature Comparison: Windows vs Linux

| Feature | Windows | Linux v1.0 (BETA) |
|---------|---------|-------------------|
| Core Editing | ✅ | ✅ |
| Multi-cursor | ✅ | ✅ |
| Split View | ✅ | ✅ |
| Code Folding | ✅ | ✅ |
| Syntax Highlighting | 80+ languages | 20+ languages |
| Macros | Advanced | Basic |
| Plugins | Extensive | Not yet |
| Themes | Many | 4 themes |
| Session Management | ✅ | ✅ |
| Auto-save | ✅ | ✅ |

**Overall:** ~70% feature parity with Windows Notepad++

### What's Next (v1.1+)

- Additional syntax highlighting languages
- Plugin architecture
- Function list panel
- Advanced auto-completion
- Document map
- Print support

### Feedback & Contributions

Please report issues and suggest features on GitHub:  
https://github.com/lord3nd3r/notepad-plus-plus/issues

Contributions are welcome! See CONTRIBUTING.md for guidelines.

---

**Thank you for trying Notepad++ Linux GTK v1.0 (BETA)!**

*Built with ❤️ by Kristopher Craig for the Linux community*
