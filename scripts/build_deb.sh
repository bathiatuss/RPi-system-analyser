#!/bin/bash

# Build script for creating .deb package
# Usage: ./build_deb.sh

set -e

echo "Building system-analyser Debian package..."

# Clean previous builds
echo "Cleaning previous builds..."
make clean 2>/dev/null || true
rm -rf debian/system-analyser
rm -f ../*.deb ../*.buildinfo ../*.changes

# Build the package
echo "Building package..."
dpkg-buildpackage -us -uc -b

echo ""
echo "Build complete! Package created:"
ls -lh ../*.deb

echo ""
echo "To install locally: sudo dpkg -i ../system-analyser_*.deb"
echo "To fix dependencies: sudo apt-get install -f"
