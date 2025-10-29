#!/bin/bash

# Build script for Nesca v2r - All versions
# Builds both console and GUI versions

set -e  # Exit on error

echo "=========================================================="
echo "  Nesca v2r - Universal Build Script"
echo "=========================================================="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
BUILD_CONSOLE=true
BUILD_GUI=true
CLEAN=false
DEBUG=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --console-only)
            BUILD_GUI=false
            shift
            ;;
        --gui-only)
            BUILD_CONSOLE=false
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --debug)
            DEBUG=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --console-only    Build only console version"
            echo "  --gui-only        Build only GUI version"
            echo "  --clean           Clean before building"
            echo "  --debug           Build in debug mode"
            echo "  --help, -h        Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Build both versions"
            echo "  $0 --console-only     # Build only console"
            echo "  $0 --gui-only --clean # Build GUI with clean"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Check dependencies
check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for qmake
    if ! command -v qmake &> /dev/null; then
        print_error "qmake not found. Please install Qt development packages."
        echo "  Ubuntu/Debian: sudo apt-get install qtbase5-dev qtbase5-dev-tools"
        echo "  Arch:          sudo pacman -S qt5-base"
        echo "  macOS:         brew install qt5"
        exit 1
    fi
    
    QT_VERSION=$(qmake -v | grep "Using Qt version" | awk '{print $4}')
    print_success "Qt version: $QT_VERSION"
    
    # Check for required libraries
    local missing_libs=()
    
    if ! pkg-config --exists libssh 2>/dev/null; then
        missing_libs+=("libssh")
    fi
    
    if ! pkg-config --exists openssl 2>/dev/null; then
        missing_libs+=("openssl")
    fi
    
    if ! pkg-config --exists libcurl 2>/dev/null; then
        missing_libs+=("libcurl")
    fi
    
    if [ ${#missing_libs[@]} -ne 0 ]; then
        print_warning "Missing libraries: ${missing_libs[*]}"
        echo "Please install them for full functionality:"
        echo "  Ubuntu/Debian: sudo apt-get install libssh-dev libssl-dev libcurl4-openssl-dev"
        echo "  Arch:          sudo pacman -S libssh openssl curl"
    else
        print_success "All required libraries found"
    fi
    
    echo
}

# Build console version
build_console() {
    print_info "Building console version..."
    
    if [ "$CLEAN" = true ]; then
        print_info "Cleaning console build..."
        make clean 2>/dev/null || true
    fi
    
    # Generate Makefile
    if [ "$DEBUG" = true ]; then
        qmake CONFIG+=debug nesca.pro
        print_info "Building in DEBUG mode"
    else
        qmake CONFIG+=release nesca.pro
        print_info "Building in RELEASE mode"
    fi
    
    if [ $? -ne 0 ]; then
        print_error "qmake failed for console version"
        exit 1
    fi
    
    # Build
    local cpu_count=$(nproc 2>/dev/null || echo 4)
    make -j$cpu_count
    
    if [ $? -eq 0 ]; then
        print_success "Console version built successfully"
        if [ -f nesca ]; then
            ls -lh nesca
        fi
    else
        print_error "Console build failed"
        return 1
    fi
}

# Build GUI version
build_gui() {
    print_info "Building GUI version..."
    
    # Clean if requested
    if [ "$CLEAN" = true ]; then
        print_info "Cleaning GUI build..."
        rm -rf build_gui
        rm -f nesca-gui
    fi
    
    # Create build directory
    mkdir -p build_gui
    cd build_gui
    
    # Generate Makefile
    if [ "$DEBUG" = true ]; then
        qmake CONFIG+=debug ../nesca_gui.pro
        print_info "Building in DEBUG mode"
    else
        qmake CONFIG+=release ../nesca_gui.pro
        print_info "Building in RELEASE mode"
    fi
    
    if [ $? -ne 0 ]; then
        print_error "qmake failed for GUI version"
        exit 1
    fi
    
    # Build
    local cpu_count=$(nproc 2>/dev/null || echo 4)
    make -j$cpu_count
    
    if [ $? -ne 0 ]; then
        print_error "GUI build failed"
        cd ..
        return 1
    fi
    
    # Move executable to root
    if [ -f nesca-gui ]; then
        mv nesca-gui ..
        cd ..
        print_success "GUI version built successfully"
        ls -lh nesca-gui
    else
        cd ..
        print_error "Executable not found"
        return 1
    fi
}

# Main execution
main() {
    local start_time=$(date +%s)
    
    # Change to script directory
    cd "$(dirname "$0")"
    
    # Check dependencies
    check_dependencies
    
    # Build console version
    if [ "$BUILD_CONSOLE" = true ]; then
        echo
        print_info "=== Building Console Version ==="
        echo
        
        if build_console; then
            CONSOLE_SUCCESS=true
        else
            CONSOLE_SUCCESS=false
        fi
    else
        CONSOLE_SUCCESS=false
    fi
    
    # Build GUI version
    if [ "$BUILD_GUI" = true ]; then
        echo
        print_info "=== Building GUI Version ==="
        echo
        
        if build_gui; then
            GUI_SUCCESS=true
        else
            GUI_SUCCESS=false
        fi
    else
        GUI_SUCCESS=false
    fi
    
    # Summary
    echo
    echo "=========================================================="
    print_info "Build Summary"
    echo "=========================================================="
    
    if [ "$BUILD_CONSOLE" = true ]; then
        if [ "$CONSOLE_SUCCESS" = true ]; then
            print_success "Console version: SUCCESS"
            echo "  Executable: ./nesca"
        else
            print_error "Console version: FAILED"
        fi
    fi
    
    if [ "$BUILD_GUI" = true ]; then
        if [ "$GUI_SUCCESS" = true ]; then
            print_success "GUI version: SUCCESS"
            echo "  Executable: ./nesca-gui"
        else
            print_error "GUI version: FAILED"
        fi
    fi
    
    # Calculate build time
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    local minutes=$((duration / 60))
    local seconds=$((duration % 60))
    
    echo
    print_info "Total build time: ${minutes}m ${seconds}s"
    echo
    
    # Exit code
    if [ "$BUILD_CONSOLE" = true ] && [ "$BUILD_GUI" = true ]; then
        if [ "$CONSOLE_SUCCESS" = true ] && [ "$GUI_SUCCESS" = true ]; then
            print_success "All builds completed successfully!"
            exit 0
        else
            print_error "Some builds failed"
            exit 1
        fi
    elif [ "$BUILD_CONSOLE" = true ]; then
        if [ "$CONSOLE_SUCCESS" = true ]; then
            exit 0
        else
            exit 1
        fi
    elif [ "$BUILD_GUI" = true ]; then
        if [ "$GUI_SUCCESS" = true ]; then
            exit 0
        else
            exit 1
        fi
    fi
}

# Run main function
main

