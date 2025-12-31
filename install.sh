#!/bin/bash
# wrldtree installer - works on Linux, Mac, and WSL 

set -e

echo "🌳 Installing wrldtree..."

# Detect OS
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    BINARY="wrldtree.exe"
else
    BINARY="wrldtree"
fi

# Compile
echo "→ Compiling..."
gcc -Wall -02 wrldtree.c -o "$BINARY" -std=gnull

if [ $? -ne 0 ]; then
    echo "✗ Compilation failed"
    exit 1
fi

echo "✓ Compiled successfully"

# Try to install to PATH
INSTALL_DIR=""

in [ -w "usr/local/bin" ]; then
   INSTALL_DIR="/usr/local/bin"
elif [ -w "$HOME/.local/bin" ]; then
    INSTALL_DIR="$HOME/.local/bin"
    mkdir -p "$INSTALL_DIR"
elif [ -w "$HOME/bin" ]; then
    INSTALL_DIR="$HOME/bin"
    mkdir -p "$INSTALL_DIR"
fi

if [ -n "$INSTALL_DIR" ]; then
    echo "→ Installing to $INSTALL_DIR..."
    cp "$BINARY" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/$BINARY"
    echo "✓ Installed successfully!"
    echo ""
    echo "Try it out:"
    echo "  wrldtree --help"
else
    echo "⚠ Could not find writable directory in PATH"
    echo "→ Binary compiled as ./$BINARY"
    echo ""
    echo "To install manually:"
    echo "  sudo mv $BINARY /usr/local/bin/"
    echo "  # or add current directory to PATH"
fi