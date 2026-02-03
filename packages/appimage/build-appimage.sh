#!/bin/bash
# AppImage build script for Notepad++ GTK
# This creates a portable AppImage that runs on any Linux distribution

set -e

echo "Building Notepad++ AppImage..."

# Create AppDir structure
APPDIR="notepad-plus-plus.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/pixmaps"
mkdir -p "$APPDIR/usr/lib"

# Build the application
cd ..
make clean
make

# Copy binary
cp build/linux-gtk-prototype/notepad++ "$APPDIR/usr/bin/"

# Copy desktop file and icon
cp notepad-plus-plus.desktop "$APPDIR/usr/share/applications/"
cp notepad-plus-plus.png "$APPDIR/usr/share/pixmaps/"

# Create AppRun script
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin/:${HERE}/usr/sbin/:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib/:${HERE}/usr/lib/i386-linux-gnu/:${HERE}/usr/lib/x86_64-linux-gnu/:${LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="${HERE}/usr/share/:${XDG_DATA_DIRS}"
exec "${HERE}/usr/bin/notepad++" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Create desktop file in root
cp notepad-plus-plus.desktop "$APPDIR/"
cp notepad-plus-plus.png "$APPDIR/"

# Download appimagetool if not present
if [ ! -f "appimagetool-x86_64.AppImage" ]; then
    echo "Downloading appimagetool..."
    wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x appimagetool-x86_64.AppImage
fi

# Build AppImage
echo "Creating AppImage..."
./appimagetool-x86_64.AppImage "$APPDIR" "Notepad++-1.0-beta-x86_64.AppImage"

echo "AppImage created: Notepad++-1.0-beta-x86_64.AppImage"
echo "You can now run: ./Notepad++-1.0-beta-x86_64.AppImage"
