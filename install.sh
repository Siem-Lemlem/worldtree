#!/bin/bash
set -e

echo "🌳 Installing wrldtree..."

# Detect OS
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    BINARY="wrldtree.exe"
else
    BINARY="wrldtree"
fi

echo "→ Compiling..."
gcc -Wall -O2 wrldtree.c -o "$BINARY" -std=gnu11

INSTALL_DIR=""
if [ -w "/usr/local/bin" ]; then
    INSTALL_DIR="/usr/local/bin"
elif [ -w "$HOME/.local/bin" ]; then
    INSTALL_DIR="$HOME/.local/bin"
    mkdir -p "$INSTALL_DIR"
fi

if [ -n "$INSTALL_DIR" ]; then
    echo "→ Installing to $INSTALL_DIR..."
    cp "$BINARY" "$INSTALL_DIR/"
    chmod +x "$INSTALL_DIR/$BINARY"
    echo "✓ Installed successfully!"
else
    echo "⚠ No writable directory found in PATH. Binary is at ./$BINARY"
fi