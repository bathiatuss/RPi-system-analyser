#!/bin/bash

# Quick build script for testing
# Builds the application without creating a package

set -e

echo "Building system-analyser..."

# Clean
make clean

# Build
make

echo ""
echo "Build complete! Binary: ./system-analyser"
echo "To run: ./system-analyser"
echo "To install system-wide: sudo make install"
