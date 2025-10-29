#!/bin/bash

# Quick build script - simple wrapper for build_all.sh
# Usage: ./build.sh [options]

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Call build_all.sh with all arguments
exec "$SCRIPT_DIR/build_all.sh" "$@"

