#!/bin/bash

# Script for building Windows binary using cross-compilation
# Supports MXE, mingw-w64, or Docker

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

# Check if we're using Docker
USE_DOCKER=false
if [ "$1" == "--docker" ]; then
    USE_DOCKER=true
fi

if [ "$USE_DOCKER" = true ]; then
    print_info "Building Windows binary using Docker..."
    
    if ! command -v docker &> /dev/null; then
        print_error "Docker not found! Please install Docker first."
        exit 1
    fi
    
    # Build Docker image
    docker build -f Dockerfile.windows -t nesca-windows-builder .
    
    # Run build in container
    docker run --rm -v "$(pwd):/app" nesca-windows-builder /bin/bash -c "
        cd /app && \
        bash build_windows_local.sh
    "
    
    exit 0
fi

# Check for MXE (M cross environment) - preferred method
MXE_PATH=""
if [ -d "/opt/mxe" ]; then
    MXE_PATH="/opt/mxe"
elif [ -d "$HOME/mxe" ]; then
    MXE_PATH="$HOME/mxe"
elif [ -d "/usr/lib/mxe" ]; then
    MXE_PATH="/usr/lib/mxe"
fi

if [ -n "$MXE_PATH" ] && [ -f "${MXE_PATH}/usr/x86_64-w64-mingw32.static/qt5/bin/qmake" ]; then
    print_info "Using MXE for cross-compilation..."
    MINGW_GCC="x86_64-w64-mingw32.static"
    MINGW_PREFIX="${MXE_PATH}/usr/${MINGW_GCC}"
    MINGW_QMAKE="${MINGW_PREFIX}/qt5/bin/qmake"
    export PATH="${MXE_PATH}/usr/bin:${PATH}"
    print_success "Found MXE at ${MXE_PATH}"
elif [ -n "$MXE_PATH" ] && [ -f "${MXE_PATH}/usr/x86_64-w64-mingw32.shared/qt5/bin/qmake" ]; then
    print_info "Using MXE (shared) for cross-compilation..."
    MINGW_GCC="x86_64-w64-mingw32.shared"
    MINGW_PREFIX="${MXE_PATH}/usr/${MINGW_GCC}"
    MINGW_QMAKE="${MINGW_PREFIX}/qt5/bin/qmake"
    export PATH="${MXE_PATH}/usr/bin:${PATH}"
    print_success "Found MXE at ${MXE_PATH}"
else
    # Try local mingw-w64
    print_info "Checking for local mingw-w64 installation..."
    
    # Check for mingw-w64
    MINGW_GCC=""
    if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
        MINGW_GCC="x86_64-w64-mingw32"
        print_success "Found mingw-w64 (x86_64)"
    elif command -v i686-w64-mingw32-g++ &> /dev/null; then
        MINGW_GCC="i686-w64-mingw32"
        print_warning "Found mingw-w64 (i686, 32-bit)"
    else
        print_error "mingw-w64 not found and MXE not found!"
        echo ""
        echo "For Windows cross-compilation, you need one of:"
        echo ""
        echo "Option 1: Install MXE (recommended)"
        echo "  git clone https://github.com/mxe/mxe.git ~/mxe"
        echo "  cd ~/mxe && make qtbase"
        echo ""
        echo "Option 2: Install mingw-w64"
        echo "  Arch Linux: sudo pacman -S mingw-w64-gcc"
        echo "  Ubuntu/Debian: sudo apt-get install mingw-w64"
        echo ""
        echo "Option 3: Use Docker"
        echo "  ./build_windows.sh --docker"
        exit 1
    fi
    
    MINGW_PREFIX="/usr/${MINGW_GCC}"
    MINGW_QMAKE=""
    
    # Check for Qt installation
    if [ -d "${MINGW_PREFIX}/lib/qt5" ]; then
        MINGW_QMAKE="${MINGW_PREFIX}/lib/qt5/bin/qmake"
    elif [ -d "/usr/lib/qt5" ]; then
        # Try system Qt (won't work, but will give better error)
        print_warning "System Qt found, but need Windows Qt. Consider using MXE."
        MINGW_QMAKE=""
    fi
    
    if [ -z "$MINGW_QMAKE" ]; then
        print_error "Qt for Windows not found!"
        echo ""
        echo "You need Qt5 for Windows. Best option: Install MXE"
        echo "  git clone https://github.com/mxe/mxe.git ~/mxe"
        echo "  cd ~/mxe && make qtbase"
        exit 1
    fi
fi

print_info "Using Qt: ${MINGW_QMAKE}"
print_info "Using compiler: ${MINGW_GCC}"

# Create build directory
BUILD_DIR="build_windows"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Clean previous build
print_info "Cleaning previous build..."
rm -rf * || true

# Configure Qt project for Windows
print_info "Configuring project for Windows..."

# Set up environment for cross-compilation
export PKG_CONFIG_PATH="${MINGW_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH}"
if command -v ${MINGW_GCC}-pkg-config &> /dev/null; then
    export PKG_CONFIG="${MINGW_GCC}-pkg-config"
fi

# Configure qmake for Windows
print_info "Running qmake..."
${MINGW_QMAKE} \
    -spec win32-g++ \
    "QMAKE_CC=${MINGW_GCC}-gcc" \
    "QMAKE_CXX=${MINGW_GCC}-g++" \
    "QMAKE_LINK=${MINGW_GCC}-g++" \
    "QMAKE_STRIP=${MINGW_GCC}-strip" \
    "QMAKE_RC=${MINGW_GCC}-windres" \
    CONFIG+=release \
    "../nesca.pro"

if [ $? -ne 0 ]; then
    print_error "qmake configuration failed!"
    exit 1
fi

# Build
print_info "Building..."
CPU_COUNT=$(nproc 2>/dev/null || echo 4)
make -j${CPU_COUNT}

if [ $? -eq 0 ]; then
    print_success "Build completed!"
    
    # Find executable
    EXE=""
    if [ -f "release/nesca.exe" ]; then
        EXE="release/nesca.exe"
    elif [ -f "debug/nesca.exe" ]; then
        EXE="debug/nesca.exe"
    elif [ -f "nesca.exe" ]; then
        EXE="nesca.exe"
    else
        print_warning "Executable not found in expected location, searching..."
        EXE=$(find . -name "nesca.exe" -type f | head -1)
    fi
    
    if [ -n "$EXE" ] && [ -f "$EXE" ]; then
        print_success "Executable: ${BUILD_DIR}/${EXE}"
        ls -lh "$EXE"
        
        # Copy to release directory
        mkdir -p ../release_windows
        cp "$EXE" ../release_windows/nesca.exe
        print_info "Copied to: release_windows/nesca.exe"
        
        # Try to copy required DLLs
        print_info "Checking for required DLLs..."
        if [ -n "$MXE_PATH" ]; then
            # MXE case - DLLs are in bin directory
            DLL_DIR="${MINGW_PREFIX}/bin"
            if [ -d "$DLL_DIR" ]; then
                print_info "Copying DLLs from MXE..."
                cp -v ${DLL_DIR}/*.dll ../release_windows/ 2>/dev/null || true
            fi
        fi
        
        cd ..
        print_success "Windows build completed successfully!"
        print_info "Output: release_windows/nesca.exe"
    else
        print_error "Executable not found!"
        find . -name "*.exe" -type f || true
        cd ..
        exit 1
    fi
else
    print_error "Build failed!"
    cd ..
    exit 1
fi

