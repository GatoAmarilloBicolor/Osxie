# Osxie - macOS Application Compatibility Layer

<p align="center">
  <h1>🍎 OSXIE 🍎</h1>
  <h3>Open System X Integration Environment</h3>
  <i>Run macOS applications natively on Linux</i>
</p>

---

## What is Osxie?

Osxie is an advanced fork of Darling that provides a complete macOS compatibility layer for Linux. It allows you to run macOS applications, including GUI applications like iTerm2 and command-line tools from Homebrew, without virtualization.

## Key Features

✅ **Full GUI Support** - Complete AppKit/Cocoa implementation via Cocotron
✅ **WindowServer** - Native X11 backend for macOS window management  
✅ **Security Framework** - Keychain and authorization services
✅ **WebKit** - Basic web rendering for modern applications
✅ **Homebrew Compatible** - Run brew and install macOS packages
✅ **iTerm2 Support** - Full terminal emulator compatibility


## What's New in Osxie

Osxie includes significant improvements over the original Darling:

- **Complete GUI Stack**: Full implementation of AppKit, CoreGraphics, QuartzCore
- **Enhanced CATiledLayer**: Proper tiled rendering support for iTerm2
- **WindowServer**: Complete window management with X11 backend
- **Security Framework**: Full keychain and authorization implementation
- **WebKit Implementation**: Basic but functional WebView support

- **Independent Installation**: Installs to `/usr/local/libexec/osxie` (separate from Darling)

## Quick Start

### Installation

```bash
# Clone Osxie
git clone https://github.com/yourusername/osxie
cd osxie

# Build
mkdir build && cd build
cmake .. -DCOMPONENTS=all -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Install (requires root)
sudo make install
```

### Usage

```bash
# Start Osxie shell
osxie shell

# Run commands
osxie shell -c "echo Hello from macOS"

# Install Homebrew
osxie shell -c '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'

# Use brew
osxie shell -c "brew install wget"

# Run iTerm2
osxie shell -c "/Applications/iTerm.app/Contents/MacOS/iTerm2"
```

### GARS - Automatic Issue Resolution

When you encounter compatibility issues, GARS can automatically fix them:

```bash
# Run GARS on an error
osxie gars analyze error.log

# GARS will:
# 1. Parse the error
# 2. Understand the issue
# 3. Research solutions
# 4. Design a fix
# 5. Implement it
# 6. Verify it works
# 7. Learn for future
```

## Supported Applications

### Fully Supported ✅
- Homebrew and its packages
- Command-line tools (git, node, python, ruby)
- Basic GUI applications

### In Progress 🚧
- iTerm2 (90% working)
- VSCode
- Discord
- Slack

### Planned 📋
- Xcode
- Safari
- Native macOS games

## Architecture

```
Application Layer
    ├── macOS Binary (.app or CLI tool)
    └── Frameworks (AppKit, Foundation, etc.)
         
Osxie Layer
    ├── WindowServer (X11/Wayland backend)
    ├── Cocotron (GUI frameworks)
    ├── Security Framework
    ├── WebKit (Basic implementation)
    └── GARS (Automatic fixes)
         
Linux Layer
    ├── X11/Wayland
    ├── ALSA/PulseAudio
    └── Linux Kernel
```

## Contributing

Osxie is open source and welcomes contributions! Areas where we need help:

- Improving WebKit implementation
- Adding Metal support via Vulkan
- Implementing missing frameworks
- Testing with more applications
- Documentation and tutorials

## License

Osxie is licensed under the GNU General Public License v3.0, maintaining compatibility with the original Darling project.

## Acknowledgments

Osxie is based on the excellent work of the Darling project team. We're grateful for their pioneering efforts in macOS compatibility on Linux.

---

**Osxie** - Because your favorite macOS apps shouldn't be locked to one platform 🚀
