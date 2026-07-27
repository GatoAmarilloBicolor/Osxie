#!/bin/bash
# Osxie installation script

set -e

echo "Installing Osxie to /usr/local..."

cd /home/fenix/src/Osxie/build

# Install Osxie
cmake --install . || exit 1

# Verify installation
if [ -f /usr/local/bin/osxie ]; then
    echo "✓ Osxie installed successfully"
    echo "  Binary: /usr/local/bin/osxie"
    echo "  Prefix: /usr/local/libexec/osxie"
    
    # Set permissions
    chmod u+s /usr/local/bin/osxie
    echo "✓ Set SUID permission on osxie binary"
    
    echo ""
    echo "Installation complete!"
    echo "You can now use: osxie shell"
else
    echo "✗ Installation failed"
    exit 1
fi