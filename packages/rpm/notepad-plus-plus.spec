Name:           notepad-plus-plus
Version:        1.0
Release:        0.1.beta%{?dist}
Summary:        Native Linux port of Notepad++ using GTK3

License:        GPLv3+
URL:            https://github.com/lord3nd3r/notepad-plus-plus
Source0:        %{name}-%{version}-beta.tar.gz

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++
BuildRequires:  gtk3-devel
BuildRequires:  pkgconfig

Requires:       gtk3

%description
Notepad++ is a powerful text editor ported to Linux using GTK3.
This port provides approximately 70% feature parity with the Windows version.

Features include:
- Full core editing functionality (undo/redo, cut/copy/paste)
- Multi-cursor editing and column/rectangular selection
- 20+ syntax highlighting languages
- 4 color themes (Dark, Light, Solarized Dark/Light)
- Session management with auto-save
- 80+ keyboard shortcuts
- Code folding and split view
- Macro recording and playback
- Find/Replace with regex support

%prep
%autosetup -n %{name}-%{version}-beta

%build
%cmake
%cmake_build

%install
%cmake_install
install -D -m 644 notepad-plus-plus.desktop %{buildroot}%{_datadir}/applications/notepad-plus-plus.desktop
install -D -m 644 notepad-plus-plus.png %{buildroot}%{_datadir}/pixmaps/notepad-plus-plus.png

%files
%license LICENSE
%doc README.md CHANGELOG.md RELEASE_NOTES.md
%{_bindir}/notepad++
%{_datadir}/applications/notepad-plus-plus.desktop
%{_datadir}/pixmaps/notepad-plus-plus.png

%changelog
* Sun Feb 02 2026 Kristopher Craig <lord3nd3r@github.com> - 1.0-0.1.beta
- Initial beta release
- ~70% feature parity with Windows Notepad++
- Complete core editing functionality
- Multi-cursor editing support
- 20+ syntax highlighting languages
