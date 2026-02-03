#!/bin/bash
# Debian package build script for Notepad++
# Creates a .deb package for Ubuntu/Debian-based distributions

set -e

echo "Building Debian package for Notepad++..."

# Create debian directory in project root
cd ../..
rm -rf debian
cp -r packages/debian debian

# Build the package
echo "Running dpkg-buildpackage..."
dpkg-buildpackage -us -uc -b

echo ""
echo "Debian package built successfully!"
echo "Package file: ../notepad-plus-plus_1.0~beta-1_amd64.deb"
echo ""
echo "To install:"
echo "  sudo dpkg -i ../notepad-plus-plus_1.0~beta-1_amd64.deb"
echo "  sudo apt-get install -f  # if dependencies are missing"
