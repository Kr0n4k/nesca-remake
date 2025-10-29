#!/bin/bash

# Build script for Nesca v2r GUI version

echo "======================================"
echo "  Nesca v2r GUI - Build Script"
echo "======================================"
echo

# Check if qmake is available
if ! command -v qmake &> /dev/null; then
    echo "Error: qmake not found. Please install Qt development packages."
    echo "  On Ubuntu/Debian: sudo apt-get install qtbase5-dev qtbase5-dev-tools"
    echo "  On Arch: sudo pacman -S qt5-base"
    echo "  On macOS: brew install qt5"
    exit 1
fi

# Check Qt version
QT_VERSION=$(qmake -v | grep "Using Qt version" | awk '{print $4}')
echo "Qt version: $QT_VERSION"

# Clean previous build
echo "Cleaning previous build..."
make clean 2>/dev/null || true
rm -f nesca-gui
rm -rf build_gui 2>/dev/null

# Create build directory
mkdir -p build_gui
cd build_gui

# Generate Makefile
echo "Generating Makefile..."
qmake ../nesca_gui.pro

if [ $? -ne 0 ]; then
    echo "Error: qmake failed"
    exit 1
fi

# Build
echo "Building GUI application..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Error: Build failed"
    exit 1
fi

# Move executable to root
if [ -f nesca-gui ]; then
    mv nesca-gui ..
    echo
    echo "======================================"
    echo "  Build successful!"
    echo "======================================"
    echo "Executable: ./nesca-gui"
    echo
else
    echo "Error: Build completed but executable not found"
    exit 1
fi

