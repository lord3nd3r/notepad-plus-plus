# Notepad++ Linux Packages

This directory contains packaging scripts and metadata for distributing Notepad++ on various Linux distributions.

## 📦 Available Package Formats

### 1. **AppImage** (Universal - Any Linux Distribution)
**Location:** `appimage/`

The AppImage format creates a portable executable that runs on any modern Linux distribution without installation.

**Build Instructions:**
```bash
cd packages/appimage
./build-appimage.sh
```

**Output:** `Notepad++-1.0-beta-x86_64.AppImage`

**Usage:**
```bash
chmod +x Notepad++-1.0-beta-x86_64.AppImage
./Notepad++-1.0-beta-x86_64.AppImage
```

**Advantages:**
- ✅ Runs on any Linux distribution (Ubuntu, Fedora, Arch, etc.)
- ✅ No installation required
- ✅ Self-contained (includes all dependencies)
- ✅ Can be run from USB drive
- ✅ Perfect for testing

---

### 2. **DEB Package** (Debian/Ubuntu/Mint)
**Location:** `debian/`

Creates a `.deb` package for Debian-based distributions (Ubuntu, Debian, Linux Mint, Pop!_OS, etc.)

**Build Requirements:**
```bash
sudo apt-get install build-essential debhelper cmake libgtk-3-dev pkg-config
```

**Build Instructions:**
```bash
cd packages/debian
./build-deb.sh
```

**Output:** `../notepad-plus-plus_1.0~beta-1_amd64.deb`

**Installation:**
```bash
sudo dpkg -i notepad-plus-plus_1.0~beta-1_amd64.deb
sudo apt-get install -f  # Install any missing dependencies
```

**Uninstallation:**
```bash
sudo apt-get remove notepad-plus-plus
```

**Advantages:**
- ✅ Native package manager integration
- ✅ Automatic dependency resolution
- ✅ Easy updates via apt
- ✅ System-wide installation

---

### 3. **RPM Package** (Fedora/RHEL/CentOS/openSUSE)
**Location:** `rpm/`

Creates an `.rpm` package for Red Hat-based distributions (Fedora, RHEL, CentOS, Rocky Linux, AlmaLinux, openSUSE, etc.)

**Build Requirements:**
```bash
# Fedora/RHEL/CentOS
sudo dnf install rpm-build cmake gcc-c++ gtk3-devel

# openSUSE
sudo zypper install rpm-build cmake gcc-c++ gtk3-devel
```

**Build Instructions:**
```bash
cd packages/rpm
./build-rpm.sh
```

**Output:** `~/rpmbuild/RPMS/x86_64/notepad-plus-plus-1.0-0.1.beta.*.rpm`

**Installation:**
```bash
# Fedora/RHEL/CentOS
sudo dnf install ~/rpmbuild/RPMS/x86_64/notepad-plus-plus-*.rpm

# openSUSE
sudo zypper install ~/rpmbuild/RPMS/x86_64/notepad-plus-plus-*.rpm

# Generic RPM
sudo rpm -ivh ~/rpmbuild/RPMS/x86_64/notepad-plus-plus-*.rpm
```

**Uninstallation:**
```bash
sudo dnf remove notepad-plus-plus  # Fedora
sudo zypper remove notepad-plus-plus  # openSUSE
```

**Advantages:**
- ✅ Native package manager integration
- ✅ Automatic dependency resolution
- ✅ Easy updates via dnf/zypper
- ✅ System-wide installation

---

## 🧪 Testing Packages

### Testing AppImage
1. Build the AppImage
2. Run it directly - no installation needed
3. Test all features
4. Works across different distributions

### Testing DEB Package
1. Build the package
2. Install on a Debian/Ubuntu system (or VM)
3. Try installing, running, and uninstalling
4. Verify desktop integration (menu entry, file associations)

### Testing RPM Package
1. Build the package
2. Install on a Fedora/RHEL system (or VM)
3. Try installing, running, and uninstalling
4. Verify desktop integration

---

## 📋 Package Contents

All packages include:
- ✅ `notepad++` binary
- ✅ Desktop file (`.desktop`)
- ✅ Application icon (PNG)
- ✅ Documentation (README, CHANGELOG, RELEASE_NOTES)
- ✅ Desktop integration
- ✅ MIME type associations

---

## 🔧 Package Metadata

- **Version:** 1.0-beta
- **License:** GPL-3.0
- **Maintainer:** Kristopher Craig
- **Homepage:** https://github.com/lord3nd3r/notepad-plus-plus
- **Dependencies:** GTK3, standard C++ libraries

---

## 📝 Notes

### AppImage Notes
- The AppImage bundles Scintilla and Lexilla statically
- Some GTK themes may not work perfectly in AppImage
- Icon may not appear in some desktop environments

### DEB Package Notes
- Requires debhelper 10 or newer
- Tested on Ubuntu 20.04+, Debian 11+
- Uses dh-cmake for building

### RPM Package Notes
- Requires rpm-build tools
- Tested on Fedora 35+, RHEL 8+
- Follows Fedora packaging guidelines

---

## 🚀 Distribution Channels

Once packages are tested, they can be distributed via:

1. **GitHub Releases** - Host AppImage, DEB, and RPM files
2. **PPA** (Ubuntu) - Create a Personal Package Archive
3. **Copr** (Fedora) - Create a community repository
4. **AUR** (Arch) - Create PKGBUILD for Arch User Repository
5. **Flatpak** - Universal package format (future consideration)
6. **Snap** - Canonical's universal package format (future consideration)

---

## 📞 Support

For packaging issues, open an issue on GitHub:
https://github.com/lord3nd3r/notepad-plus-plus/issues

---

**Developer:** Kristopher Craig  
**Built with ❤️ for the Linux community**
