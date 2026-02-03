# Contributing to Notepad++ Linux Port

Thank you for your interest in contributing to the Linux GTK port of Notepad++!

## Quick Start

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Test thoroughly
5. Commit your changes (`git commit -m 'Add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Build Setup

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake libgtk-3-dev

# Build
make

# Run
./build/linux-gtk-prototype/gtk-proto
```

## Priority Areas

We especially welcome contributions in these areas:

### High Priority
- Additional syntax highlighting languages
- Performance optimizations
- Bug fixes
- Documentation improvements

### Medium Priority
- Plugin architecture design
- Advanced auto-completion
- Function list panel
- UI/UX enhancements

### Low Priority
- Print support
- Document map
- Additional themes

## Coding Guidelines

### Style
- Follow C++17 standards
- Use GTK3 best practices
- Match existing code style
- Clear, descriptive function names
- Comment complex logic

### Testing
- Test your changes thoroughly
- Ensure no regressions
- Test on multiple Linux distributions if possible
- Include test cases for new features

### Pull Requests
- One feature or bug fix per PR
- Clear, descriptive commit messages
- Reference any related issues
- Update documentation as needed
- Keep PRs focused and manageable

## Code Structure

```
linux-gtk-prototype/
├── main_gui.cxx        # Main application (~1,700 lines)
├── CMakeLists.txt      # Build configuration
└── build/              # Build output
```

### Key Components
- **AppState**: Application state management
- **TabData**: Per-tab data structures
- **Scintilla integration**: Editor widget
- **Lexilla integration**: Syntax highlighting

## Adding Features

1. Add command handler function (`cmd_feature_name`)
2. Create menu item
3. Add keyboard shortcut
4. Connect signal handler
5. Update documentation
6. Test thoroughly

## Reporting Issues

- Check if issue already exists
- Provide clear reproduction steps
- Include system information
- Attach relevant logs if applicable

## Communication

- GitHub Issues for bugs and features
- Pull Requests for code contributions
- Be respectful and constructive

## License

All contributions are under GPL-3.0 license, same as Notepad++.

---

**Project Maintainer:** Kristopher Craig  
**Original Notepad++:** Don Ho  
**License:** GPL-3.0
