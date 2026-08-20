# Osxie - macOS Application Compatibility Layer for Linux

<p align="center">
  <h1>OSXIE</h1>
  <h3>Open System X Integration Environment</h3>
  <i>Run macOS applications natively on Linux</i>
</p>

---

## What is Osxie?

Osxie is a macOS compatibility layer that provides complete compatibility for Linux. It allows you to run macOS binaries, including GUI applications, without virtualization or emulation. The entire system installs independently to `/usr/local/libexec/osxie`.

## Key Components

- **Full GUI Support** - Complete AppKit/Cocoa implementation via Cocotron
- **WindowServer** - Native X11 backend for macOS window management  
- **Security Framework** - Keychain and authorization services
- **WebKit** - Basic web rendering for modern applications
- **Homebrew Compatible** - Run brew and install macOS packages
- **iTerm2 Support** - Full terminal emulator compatibility

## Building

### Prerequisites

- Linux x86_64 (Debian/Ubuntu/Fedora/Arch)
- clang >= 13
- cmake >= 3.13
- X11 development libraries
- libpng, libjpeg, libtiff, libgif, freetype, fontconfig, cairo
- FFmpeg libraries (libavcodec, libavformat, libavutil)
- PulseAudio development libraries
- OpenGL development libraries

### Build Instructions

```bash
# Clone
git clone <repo-url> Osxie
cd Osxie

# Configure (without WebKit/JSC due to upstream LLInt build issues)
mkdir build && cd build
cmake .. -DCOMPONENTS="stock,cli_extra,cli_dev_gui_stubs" -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Install (requires root via pkexec)
make install
```

### Installing All Components (including WebKit/JSC)

When WebKit/JSC build issues are resolved:

```bash
cmake .. -DCOMPONENTS=all -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
make install
```

## Usage

```bash
# Start an interactive shell
osxie shell

# Run a command
osxie shell -c "echo Hello from Osxie"

# Default prefix: ~/.osxie
# Custom prefix:
osxie shell --prefix=/path/to/prefix
```

## Changelog

### Rename: Darling -> Osxie

The entire codebase was systematically renamed from Darling to Osxie:

- **Root CMakeLists.txt**: Project renamed to `osxie`, all install paths changed to `libexec/osxie`
- **Binary renamed**: `darling` -> `osxie`
- **Source files**: `darling.c` -> `osxie.c`, `darling.h` -> `osxie.h` in `src/startup/`
- **Config header**: `osixie-config.h` -> `osxie-config.h` (with backward-compat symlink)
- **Type renames**: `darling_thread_create_callbacks_t` -> `osxie_thread_create_callbacks_t`
- **Struct renames**: `struct elf_calls` members renamed (e.g., `darling_thread_create` -> `osxie_thread_create`)
- **Install prefix**: `/usr/local/libexec/osxie` (independent from any Darling installation)
- **Symlinks**: `/etc/osxie`, `/Volumes/OsxieEmulatedDrive`

### Bug Fixes

- **CATiledLayer**: Implemented proper tiled drawing stub for iTerm2 compatibility (`src/external/cocotron/QuartzCore/CATiledLayer.{h,m}`)
- **elfcalls**: Fixed struct member references (`osxie_thread_create`, `osxie_thread_terminate`, `osxie_thread_get_stack`) in XNU emulation layer
- **BSD thread creation**: Updated `bsdthread_create.c` and `workq_kernreturn.c` to use `osxie_thread_create_callbacks` type
- **SDK headers**: Updated `elfcalls_wrapper.h` in SDK to use `osxie_thread_create_callbacks_t`
- **Release builds**: Added `-O2 -DNDEBUG -fno-strict-aliasing` flags
- **osixie-config.h**: Created backward-compat symlink in build dir for external submodules

## In Progress

- iTerm2 (stable, dark theme, system tray working)
- Homebrew Casks (hdiutil attach/detach working, convert pending)
- Native notifications via D-Bus (in progress)

## New Components

- **WindowServer** (`src/WindowServer/`): X11-based window management with cursor handling, display management, and event processing
- **Security framework** (`src/frameworks/Security/`): Keychain, authorization, and CommonCrypto implementations backed by OpenSSL
- **WebKit WebView** (`src/frameworks/WebKit/`): Basic WKWebView, WKWebViewConfiguration, WKPreferences implementation

## Component Exclusions

- **JavaScriptCore / WebKit**: WebKit builds with `COMPONENTS=all`. LLInt and `getNonReifiedStaticPropertyNames` fixes applied (Issues 19-20).
- **Metal**: Disabled by default (`ENABLE_METAL=OFF`). MTKView and SkyLight Metal stubs return nil gracefully. To enable, set Vulkan+LLVM on host and `-DENABLE_METAL=ON`.
- **Bluetooth**: IOBluetooth/IOBluetoothUI are 100% stubs — no BlueZ/D-Bus integration.
- **Spotlight**: Spotlight/SpotlightDaemon/SpotlightIndex are skeleton stubs — no real indexing.
- **Printing**: CUPS present at system level, but macOS PMPrintSettings/PMPrinter APIs are stubs.

## Architecture

```
Application Layer
    macOS Binary (.app or CLI tool)
    Frameworks (AppKit, Foundation, CoreGraphics, etc.)

Osxie Layer
    osxie binary (shell, prefix management)
    osxieserver (Mach/POSIX RPC server)
    dyld (Mach-O dynamic loader)
    WindowServer (X11 backend)

Linux Kernel Interface
    LKM (Osxie Kernel Module) / XNU syscall emulation
    Linux syscalls mapped to Mach/POSIX semantics

Linux Layer
    X11 / Wayland
    PulseAudio / ALSA
    OpenGL / Vulkan
    OpenSSL (for Security framework)
```

## Project Structure

```
Osxie/
  src/startup/          - osxie binary (entry point)
  src/startup/mldr/     - Mach-O loader + elfcalls bridge
  src/external/osxieserver/ - RPC server
  src/external/xnu/     - XNU kernel emulation (libsystem_kernel)
  src/external/libc/    - FreeBSD libc port
  src/external/libcxx/  - libc++ port
  src/external/CoreFoundation/ - CoreFoundation framework
  src/external/foundation/ - Foundation framework
  src/external/cocotron/ - AppKit, CoreGraphics, QuartzCore
  src/WindowServer/     - X11 window management (new)
  src/frameworks/       - Security, WebKit stubs (new)
  Developer/            - macOS SDK headers
  framework-include/    - Framework include symlinks
  cmake/                - Build system helpers
```

## Contributing

Areas where help is needed:

- Bluetooth stack (BlueZ/D-Bus integration for IOBluetooth)
- Spotlight indexing (CSSearchableIndex/NSMetadataQuery)
- Printing dialog (PMPrinter/PMPrintSettings connected to CUPS)
- Metal/Vulkan backend stabilization (Indium memory issues)
- ARM64 cross-compilation support

## License

Osxie is licensed under the GNU General Public License v3.0, maintaining compatibility with the original Osxie project.

## Acknowledgments

This project is based on the original Darling project. We are grateful for the pioneering work of the Darling team in macOS-on-Linux compatibility.

---

**Osxie** - Because your favorite macOS apps shouldn't be locked to one platform 🚀