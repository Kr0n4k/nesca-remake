#!/bin/bash

# Build script to run inside Docker container
# This script is used by build_windows.sh when --docker flag is set

set -e

print_info() {
    echo "[INFO] $1"
}

print_error() {
    echo "[ERROR] $1"
}

print_success() {
    echo "[OK] $1"
}

MINGW_GCC="x86_64-w64-mingw32"
MINGW_PREFIX="/usr/${MINGW_GCC}"

print_info "Building for Windows using ${MINGW_GCC}..."

# Install Qt for Windows (simplified - using system libraries where possible)
# Note: In a real scenario, you'd download Qt binaries or compile Qt for Windows

# Check for Qt
if [ ! -d "/opt/qt" ]; then
    print_info "Qt not found. Setting up basic build environment..."
    # We'll need to compile with available tools or download Qt binaries
fi

# Create build directory
BUILD_DIR="build_windows"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Clean
rm -rf * || true

# Configure
print_info "Configuring project..."
export PKG_CONFIG_PATH="${MINGW_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"

# For now, try basic configuration
# In real scenario, would need Qt Windows binaries
qmake -spec win32-g++ \
    "QMAKE_CC=${MINGW_GCC}-gcc" \
    "QMAKE_CXX=${MINGW_GCC}-g++" \
    "QMAKE_LINK=${MINGW_GCC}-g++" \
    "QMAKE_RC=${MINGW_GCC}-windres" \
    "../nesca.pro"

# Build
print_info "Building..."
make -j$(nproc)

if [ -f "release/nesca.exe" ] || [ -f "debug/nesca.exe" ] || [ -f "nesca.exe" ]; then
    print_success "Build completed!"
    find . -name "*.exe" -type f
else
    print_error "Build failed or executable not found"
    exit 1
fi

cd ..

