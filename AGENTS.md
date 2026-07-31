# Osxie Build Progress

## Build Status
- `system_kernel_firstpass`: **BUILDS** (as of 2026-07-30)

## Build Command
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCOMPONENTS=core -GNinja -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DTARGET_i386=1 -S src -B build_new
cmake --build build_new --target system_kernel_firstpass
```

## Issues Fixed

### 1. Include guard collision in SDK stdatomic.h
- **File**: `Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/stdatomic.h`
- **Problem**: Both the SDK's `stdatomic.h` and Clang's `stdatomic.h` used `__CLANG_STDATOMIC_H` as include guard. When building with `-nostdinc`, only the SDK version was included, but it deferred to `<stdatomic.h>` (Clang's) which was already "included" due to the shared guard, so nothing was defined.
- **Fix**: Added `!defined(OSXIE)` to the `#if` condition that checks for `__CLANG_STDATOMIC_H`, so when `OSXIE` is defined, the SDK's `stdatomic.h` uses its own definitions (the `#else` block) instead of delegating.

### 2. ld64 linker assertion failure (std::vector)
- **Problem**: ld64 crashed with `std::vector::operator[]: Assertion '__n < this->size()'` because host GCC 16's libstdc++ enables `_GLIBCXX_ASSERTIONS` in unoptimized builds.
- **Fix**: Added `-D_GLIBCXX_NO_ASSERTIONS` to ld64's CXX flags.

### 3. Text relocation in platform_firstpass
- **File**: `src/external/libplatform/libplatform/string/private/crashlog_private.h` (SDK copy too)
- **Problem**: `_gCRAnnotations` in inline assembly caused "illegal text reloc" error.
- **Fix**: Added `defined(OSXIE)` to the `#if defined(DARLING)` guard so crashlog macros are no-ops.

### 4. Missing bootstrap_cmds darling/ directory
- **Problem**: `bootstrap_cmds` submodule was forked but the `darling/` directory was never committed, leaving `migcom` unable to build.
- **Created headers** in `src/external/bootstrap_cmds/darling/include/mach/`:
  - `boolean.h` - boolean_t, TRUE/FALSE
  - `kern_return.h` - kern_return_t, KERN_SUCCESS/KERN_FAILURE
  - `message.h` - mach_msg types, port type constants, descriptors
  - `std_types.h` - natural_t, integer_t
  - `ndr.h` - NDR_record_t struct
- **Created script** `src/external/bootstrap_cmds/darling/src/mig.awk`: awk script to transform `mig.sh` template into `build-mig` with proper paths.

### 5. Missing DARLING define for migcom
- **File**: `src/external/bootstrap_cmds/CMakeLists.txt`
- **Problem**: `-xtracemig` flag support was guarded by `#ifdef DARLING` but the define was not set.
- **Fix**: Added `-DDARLING` to `add_definitions()`.

### 6. Broken nolegacy.h symlink
- **File**: `src/external/xnu/libsyscall/nolegacy.h` (symlink)
- **Problem**: Pointed to `../darling/...` which doesn't exist.
- **Fix**: Re-pointed to `../osxie/src/libsystem_kernel/extra/nolegacy.h`.

### 7. Missing emulation include path (osxie/emulation symlink)
- **File**: `src/external/xnu/osxie/src/libsystem_kernel/emulation/include/osxie/emulation` (symlink → `..`)
- **Problem**: Code includes `<osxie/emulation/common/guarded/table.h>` but the `include/` directory wasn't in the include path, and the `osxie/emulation` prefix didn't exist.
- **Fix**: Created symlink `include/osxie/emulation → ..` so `include/osxie/emulation/X` resolves to `include/X`. Added `include/` to emulation's include_directories.

### 8. Missing mach_types port type aliases
- **Files**: `src/external/xnu/osfmk/mach/mach_types.h` and `Developer/.../SDK/.../usr/include/mach/mach_types.h`
- **Problem**: MIG-generated code used `semaphore_consume_ref_t`, `thread_act_consume_ref_t`, `mem_entry_name_port_move_send_t` but no C typedef existed.
- **Fix**: Added typedefs: `semaphore_consume_ref_t = semaphore_t`, `thread_act_consume_ref_t = thread_act_t`, `mem_entry_name_port_move_send_t = mem_entry_name_port_t`.

### 9. Missing iokit include path
- **File**: `src/external/xnu/libsyscall/CMakeLists.txt`
- **Problem**: XNU's `iokit/IOKit/` headers weren't in the include path.
- **Fix**: Added `include_directories("${CURRENT_SOURCE_DIR_NO_SYMLINKS}/../iokit")`.

### 10. Missing IOKit/usb and IOKit/firewire headers
- **Created stubs** in `src/external/xnu/iokit/IOKit/`:
  - `usb/USB.h` - USB error code constants
  - `firewire/IOFireWireLib.h` - FireWire error code constants

### 11. Missing OSXIE guard in task.c
- **File**: `src/external/xnu/osxie/src/libsystem_kernel/libsyscall/mach/task.c`
- **Problem**: Workaround for missing `task_internal.h` was guarded by `#ifdef DARLING` but build uses `OSXIE`.
- **Fix**: Changed to `#if defined(DARLING) || defined(OSXIE)`.

### 12. Missing emulation include in libsyscall
- **File**: `src/external/xnu/libsyscall/CMakeLists.txt`
- **Problem**: libsyscall target needed the emulation `include/` directory for `<osxie/emulation/...>` headers.
- **Fix**: Added `include_directories("...emulation/include")`.

### 13. vchroot link failure: misspelled symbol
- **File**: `src/vchroot/vchroot.c`
- **Problem**: `vchroot` failed to link with `Undefined symbols: "___osixie_vchroot"`. `vchroot.c` declared/referenced `__osixie_vchroot` (typo, extra "i") but libsystem_kernel's emulation exports `__osxie_vchroot` / `__osxie_vchroot_expand` (see `src/external/xnu/osxie/src/libsystem_kernel/emulation/src/linux_premigration/vchroot_userspace.c`). Introduced in commit `f30104686` when renaming `__darling_vchroot`.
- **Fix**: Changed `__osixie_vchroot` → `__osxie_vchroot` in `vchroot.c` (lines 8 and 40). Verified with `cmake --build . --target vchroot`; binary now imports `___osxie_vchroot` correctly.
- **Note**: The codebase uses both spellings (`osixie` in lkm/xtrace/commpage, `osxie` in libsystem_kernel emulation and shellspawn plist). For the vchroot ABI, `osxie` is canonical since libsystem_kernel already exports it.

## Next Targets to Build
After `system_kernel_firstpass`, the next targets would be:
- `system_kernel` (the fat binary, second pass)
- Other system libraries
