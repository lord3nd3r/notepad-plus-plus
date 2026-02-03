#!/bin/bash
# RPM build script for Notepad++
# Creates an RPM package for Fedora/RHEL/CentOS

set -e

echo "Building RPM package for Notepad++..."

# Create tarball
cd ../..
VERSION="1.0-beta"
TARBALL="notepad-plus-plus-${VERSION}.tar.gz"

echo "Creating source tarball..."
tar --exclude='.git' --exclude='build' --exclude='packages' \
    -czf "/tmp/${TARBALL}" .

# Setup RPM build environment
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy files
cp "/tmp/${TARBALL}" ~/rpmbuild/SOURCES/
cp packages/rpm/notepad-plus-plus.spec ~/rpmbuild/SPECS/

# Build RPM
echo "Building RPM package..."
cd ~/rpmbuild/SPECS
rpmbuild -ba notepad-plus-plus.spec

echo ""
echo "RPM package built successfully!"
echo "Package files in: ~/rpmbuild/RPMS/x86_64/"
echo ""
echo "To install:"
echo "  sudo dnf install ~/rpmbuild/RPMS/x86_64/notepad-plus-plus-*.rpm"
echo "  or"
echo "  sudo rpm -ivh ~/rpmbuild/RPMS/x86_64/notepad-plus-plus-*.rpm"
