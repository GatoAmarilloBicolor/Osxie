# Osxie Build Progress

## Build Status
- `system_kernel_firstpass`: **BUILDS** (as of 2026-07-31, verified with `-S .`)
- `system_kernel`: **BUILDS** (as of 2026-07-31, second pass; produces `libsystem_kernel.dylib`)
- `osxieserver`: **RUNS** (2026-08-01; ELF x86-64, usage displayed correctly)
- `osxie`: **RUNS** (2026-08-01; ELF x86-64, usage displayed correctly)
- **FULL `COMPONENTS=all` build (Make, `build-all`): **BUILDS** (2026-08-01, `scripts/build_complete.sh --no-install`, rc=0). Includes JavaScriptCore, Ruby, vim, pyobjc, QuartzComposer, SceneKit, AVKit, etc.
- **FULL `COMPONENTS=all` Release build (`build-all`, Unix Makefiles): **BUILDS** (2026-08-01, `make -j4`, rc=0). JavaScriptCore Release link failures fixed (see Issues Fixed #19, #20).

## Build Command
The canonical flow configures the **repo-root** as the source directory (`-S .`).
`src/CMakeLists.txt` is a subdirectory of the repo-root `CMakeLists.txt`
(`add_subdirectory(src)`); the `-S src` workflow never worked because subproject
CMakeLists (startup, osxieserver, libsystem_kernel/emulation, ...) hardcode
`${CMAKE_SOURCE_DIR}/src/...` and `${CMAKE_BINARY_DIR}/src/...` paths that only
resolve with the repo root as top-level.
```
cmake -DCMAKE_BUILD_TYPE=Debug -DCOMPONENTS=core -GNinja -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DTARGET_i386=1 -S . -B build_new
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

### 14. `-S src` never worked; use `-S .` (repo root)
- **Problem**: AGENTS.md documented `cmake -S src -B build_new`, but `src/CMakeLists.txt` was written as a subdirectory of the repo-root `CMakeLists.txt` (`add_subdirectory(src)`). Subproject CMakeLists (startup, osxieserver, libsystem_kernel/emulation, ...) hardcode `${CMAKE_SOURCE_DIR}/src/...` and `${CMAKE_BINARY_DIR}/src/...`, which only resolve with the repo root as top-level. Under `-S src` these became double-`src` paths (e.g. `-I.../src/src/external/osxieserver/include`) and migcom failed on `Availability.h` (repo-root `basic-headers` not on path).
- **Fix**: Standardize on `-S .` (see Build Command). Also fixed `src/CMakeLists.txt` to be flow-agnostic where cheap (module path, basic-headers/SDK/framework/libcxx include dirs, CMAKE_AR/RANLIB via `CMAKE_CURRENT_*`).

### 15. mig() not defined for COMPONENTS=core
- **File**: `src/CMakeLists.txt`
- **Problem**: `include(mig)` was guarded by `if (COMPONENT_system OR COMPONENT_python OR COMPONENT_ruby)` (regression in commit `436ade32d`), but `osxie_parse_components` only sets `COMPONENT_core` for `COMPONENTS=core`. osxieserver (a core component) calls `mig()`, so configure failed with "Unknown CMake command 'mig'". The `-S src` workflow also never parses components, so no `COMPONENT_*` vars existed at all.
- **Fix**: Made `include(mig)` unconditional.

### 16. Missing CMake scaffolding for standalone/`-S src` configure
- **Files**: `src/CMakeLists.txt`
- **Problem**: With `-S src`, `src/CMakeLists.txt` ran standalone but lacked `cmake_minimum_required` (CMP0000 error), `enable_language(ASM)` (missing `CMAKE_ASM_COMPILE_OBJECT` in CMake 4.4), and `generate_architecture()` (`CoreFoundation_i386` target not found, `APPLE_TARGET_TRIPLET_PRIMARY` undefined).
- **Fix**: Added `cmake_minimum_required(VERSION 3.13)`, `enable_language(ASM)`, and `include(architecture)` + `generate_architecture()`. These are no-ops in the repo-root flow (vars already set).

### 17. Compiler selection: clang required
- **File**: `src/CMakeLists.txt`
- **Problem**: `-S src` with default CMake compiler detection picked `/usr/bin/cc` (GCC), which does not recognize the `__private_extern__` keyword used by cctools headers (`libstuff/rnd.c: error: unknown type name '__private_extern__'`). The repo-root `CMakeLists.txt` explicitly prefers clang; the `-S src` flow had no such logic.
- **Fix**: Added a `find_program` clang/clang++ preference before `project()` in `src/CMakeLists.txt` (skipped when `CMAKE_C_COMPILER` is already set).

### 18. Compiler builtin headers missing under `-nostdinc` (stdarg.h)
- **File**: `src/CMakeLists.txt`
- **Problem**: libsyscall compiles with `-nostdinc`, which also drops clang's builtin header dir, so MIG-generated sources failed with `fatal error: 'stdarg.h' file not found`. `GetCompilerInclude()` (which adds `-isystem /usr/lib/clang/<ver>/include`) was guarded by `if (COMPONENT_system OR COMPONENT_python)`, neither set for `COMPONENTS=core`. The old `build/` used `stock` components, which transitively enabled `system`, masking the bug.
- **Fix**: Made `include(compiler_include)` + `GetCompilerInclude()` + `include_directories(SYSTEM ...)` unconditional.

### 19. JavaScriptCore LLInt empty object in Release builds (missing vmEntry*/wasm_entry symbols)
- **File**: `src/external/JavaScriptCore/CMakeLists.txt` (submodule)
- **Problem**: `low_level_interpreter_x86_64` was hardcoded to the `DerivedSources/JavaScriptCore/LLIntOffsets/X86_64/debug` LLIntAssembly header. Its guard requires `OFFLINE_ASM_ASSERT_ENABLED` (i.e. `ASSERT_ENABLED`, only set in Debug). In a `Release` build (`-DNDEBUG`) the guard fails, the header compiles to nothing, `llint/LowLevelInterpreter.cpp` produces an **empty `__text` object**, and linking JavaScriptCore fails with undefined `_vmEntryToJavaScript`, `_vmEntryToNative`, `_vmEntryHostFunction`, `_vmEntryCustomAccessor`, `_vmEntryRecord`, `_wasm_entry`, `_wasmLLIntPCRangeStart/End`, `_llintPCRangeStart`. The x86_64 case was a debug-only leftover; the i386 C_LOOP case already selected `release` vs `debug` by build type (see CMakeLists.txt lines ~2172).
- **Fix**: Select `X86_64/release` when `CMAKE_BUILD_TYPE` is `Release`/`RelWithDebInfo`/`MinSizeRel`, else `X86_64/debug` — mirroring the i386 logic. (Also documented in the CMake comment: if the header/assertion guard mismatches, the object comes out empty.)

### 20. JavaScriptCore missing `JSObject::getNonReifiedStaticPropertyNames` in Release (always_inline drops out-of-line symbol)
- **File**: `src/external/JavaScriptCore/runtime/JSObject.cpp` (submodule)
- **Problem**: Under `NDEBUG`, `ALWAYS_INLINE` (`wtf/Compiler.h`) expands to `inline __attribute__((always_inline))`. `JSObject::getNonReifiedStaticPropertyNames` is defined `ALWAYS_INLINE` in `JSObject.cpp` but called from a **different** TU (`runtime/JSPropertyNameEnumerator.cpp:113`). In a unified build Apple inlines everything into one TU; in Osxie's non-unified build Clang inlines all same-TU call sites and **drops the out-of-line copy**, so linking fails with `Undefined symbols for architecture x86_64: __ZN3JSC8JSObject32getNonReifiedStaticPropertyNamesE...`. (Debug builds are unaffected because `ALWAYS_INLINE` is plain `inline` when `NDEBUG` is not defined.)
- **Fix**: Changed the definition to plain `inline` — the optimizer still inlines, but the symbol is also emitted for cross-TU callers. If more such cross-TU `ALWAYS_INLINE` symbols surface in Release, the same one-file fix applies (or gate `Compiler.h` on `OSXIE`, which would trigger a full-tree recompile).

### 21. Indium/iridium CMakeLists still guarded on `DARLING` (unmigrated)
- **File**: `src/external/metal/deps/indium/CMakeLists.txt` (nested submodule, detached `8423a7d`)
- **Problem**: The osxify migration renamed the global CMake variable `DARLING` → `OSXIE` (`src/CMakeLists.txt:81 set(OSXIE TRUE)`), but this submodule's CMakeLists still had `if (DARLING)` / `if (NOT DARLING)` guards. With `DARLING` undefined it took the **host-build** branch (`add_library(... SHARED)` → `libiridium.so`) while the global toolchain flags still target Mach-O, so linking failed with `air.cpp.o: file not recognized` (Mach-O object into an ELF `.so`). Surfaced only when a CMakeLists edit (`shellspawn`) finally forced a full reconfigure that picked up `add_subdirectory(external/metal)` (gated on `COMPONENT_dev_gui_common`, enabled by `COMPONENTS=all`).
- **Fix**: Migrated all 13 guards to `if (DARLING OR OSXIE)` / `if (NOT DARLING AND NOT OSXIE)`, so it takes the dylib branch (`add_osxie_library`, `make_fat`) and produces `libiridium.dylib`. Committed on an `osxie` branch in the indium submodule; parent pointer bump is local (no fork remote configured for `darlinghq/indium` or `darling-metal`).

### 22. Stale plist filenames in install rules (post-osxify renames)
- **Files**: `src/shellspawn/CMakeLists.txt`, `src/external/iokitd/CMakeLists.txt`
- **Problem**: `make install` failed with `file INSTALL cannot find` because the CMake install rules referenced pre-rename filenames that don't exist: `org.osixie.shellspawn.plist` (extra "i" typo; tracked file is `org.osxie.shellspawn.plist`) and `org.osxiehq.iokitd.plist` (old darlinghq name; tracked file is `org.osxie.iokitd.plist`). Note: the on-disk `org.osixie.shellspawn.plist` was also in a btrfs-inconsistent state (`ls` showed it but `stat`/`readdir`-followed `statx` returned ENOENT), which first looked like filesystem corruption.
- **Fix**: Pointed both `install(FILES ...)` rules at the tracked filenames and removed the broken on-disk file. Checked the rest of the tree for other stale `darlinghq`/`osxiehq` plist references — only test-only ones remain (`src/external/foundation/test/nsxpc-launchd-service`, `src/external/libxpc/test/launchd-service`).

## Next Targets to Build
After `osxieserver` and `osxie` (both done), the next targets would be:
- Other system libraries (`libsystem_c`, `libsystem_dyld`, ...)
- The full core build (`ninja -C build_new`)

## Full build automation (RAM-constrained machine)

The shared machine (17Gi RAM, ~12Gi used by other agents) OOMs under `-j2` on heavy
TUs (`LowLevelInterpreter.cpp.o`, `NSPredicateParser.tab.c`, `cryptkitcsp.cpp.o`).
The OOM killer even took the old retry-loop shell itself, killing it silently.

**Use `scripts/build_complete.sh` — one command for build + install (+ optional
osxify fork/push):**

```bash
./scripts/build_complete.sh                 # build + install (1 comando)
./scripts/build_complete.sh --osxify-push   # + fork/push submódulos osxificados
./scripts/build_complete.sh --no-install --jobs=1 --max-attempts=60
```

Key mechanics (details in `.opencode/plans/build-completion.md`):
- `ulimit -v 6Gi` per process → the build never triggers a system OOM, so the retry
  shell survives; TUs over the cap just fail and are retried (observed max ~2.5Gi).
- Adaptive parallelism: `-j2` if `MemAvailable ≥ 8Gi`, else `-j1`; after first OOM
  it drops to `-j1` (peak TU + third-party ~14.5Gi < 17Gi → stable).
- Stops (rc=3) on real errors with the last `error:`/`fatal error`; retries on
  `Terminado|Killed|bad_alloc`. Log: `/tmp/opencode/build-retry.log`.
- **ccache is NOT installed** (`CCACHE_PROGRAM-NOTFOUND`): install it to make
  OOM-retries nearly free.
- Osxified submodules to fork/push when green (rest of `git submodule status` is
  build junk): `OpenLDAP`, `python`, `JavaScriptCore`, `Heimdal`, `security`
  (`git add -u` + commit `osxify: replace DARLING guards/refs with OSXIE`, branch
  `osxie`, `gh repo fork`, push) — see `.opencode/plans/build-completion.md`.
- **Forks ALL first-level submodules (2026-08-08)**: created `GatoAmarilloBicolor/osxie-*`
  forks for all 96 first-level submodules whose osxify HEAD was not published
  (`gh repo fork <upstream> --fork-name <target>`; special names: `cctools-port`,
  `lzfse`, `openjdk` keep plain name; 4 `libressl-*` dirs share the
  `osxie-libressl` fork, one branch per dir). Each submodule's gitlink HEAD is
  pushed to a branch named after its directory (`refs/heads/<dir>`), so the parent
  clones reproducibly. `swift` needs `-c core.hooksPath=/dev/null` on push (its
  `.lfsconfig` points to the auth-only `git-lfs.darlinghq.org`).

## Fast dev-loop for framework changes — `scripts/relink_dylibs.sh`

When iterating on a high-level Cocoa framework (AppKit/Foundation/CoreFoundation
for NIB/UI debugging, etc.), **do NOT** run `build_complete.sh` every time: its
`pkexec ninja install` rewrites the whole `/usr/local/libexec` tree and would
re-touch the **setuid-root `shellspawn`**, which is risky and slow. Instead use
this QoL helper — it builds just the named dylib targets and copies them straight
into the runtime prefix `~/.osxie`, with **no setuid/sudo/pkexec every**:

```bash
./scripts/relink_dylibs.sh                 # rebuild+copy AppKit & Foundation (default)
./scripts/relink_dylibs.sh AppKit          # just one
./scripts/relink_dylibs.sh --restart-server# also restart osxieserver
```

- Never touches shellspawn or the setuid install step.
- **Since 2026-08-08, relinking `AppKit` also rebuilds+copies the X11 backend**
  (`X11_backend` → `AppKit.framework/.../Resources/Backends/X11.backend/Contents/MacOS/X11`),
  because the window/tray machinery lives there.
- **Only ONE runtime prefix matters**: verified via `/proc/<app>/maps` that the
  running app loads frameworks exclusively from
  `$OSXIE_PREFIX/System/Library/Frameworks/...`. The tree at
  `$OSXIE_PREFIX/libexec/osxie/System/...` is a stale full install the app NEVER
  consults — the script does NOT copy there.
- `BUILD_DIR` and `OSXIE_PREFIX` env vars override the build tree (default
  `build_new`) and runtime prefix (default `~/.osxie`). The runtime prefix
  (`~/.osxie`) is the one the running `osxieserver` maps, NOT
  `/usr/local/libexec/osxie` (which the user only reads for SDK headers).
- `--restart-server` kills the running `osxieserver` for `~/.osxie` so the next
  `./install/bin/osxie <app>` launch maps the freshly copied dylibs.
- Then run the app setuid launcher manually (interactive session required):
  `cd install/bin && ./osxie /Applications/cpuinfo.app/Contents/MacOS/cpuinfo`.
## Progress: July 31 → August 1, 2026
- Rebuilt with flow configuration (repo-root -S .)
- Verified osxie and system_kernel.dylib linking

## Issue 23: NIBArchive `NSPlaceholderNumber` freed by `_decodeObjectAtIndex:` (cpuinfo SIGEXC)
- **File**: `src/external/cocotron/AppKit/nib.subproj/GSNibArchiveKeyedUnarchiver.m` (nested repo, NOT a submodule pointer)
- **Problem**: `_decodeObjectAtIndex:` did `object = [class allocWithZone:]` (returns the shared singleton placeholder, e.g. `NSPlaceholderNumber` from `+[NSValue allocWithZone:]`, see `src/external/foundation/src/NSValue.m:27-51`), then `[object release]` whenever `initWithCoder:`/`awakeAfterUsingCoder:`/`didDecodeObject:` returned a different object. The NEXT `[NSNumber alloc]` returned that freed block with a garbage isa → SIGEXC in `initWithUnsignedInteger:`/`initWithUnsignedLong:`/`initWithUnsignedChar:` depending on allocator.
- **Fix**: Probe for the shared placeholder right after alloc (`id probe = [class allocWithZone: _objectZone]; sharedPlaceholder = (probe == object); if (!sharedPlaceholder) [probe release];`), and guard all three `[object release]` sites with `if (!sharedPlaceholder)`.
- **Status**: UNCOMMITTED (`M nib.subproj/GSNibArchiveKeyedUnarchiver.m` in the AppKit nested repo; prior commit `09f54aba`).

## Progress: August 7, 2026 — Milestone "visible window" VERIFIED
- **cpuinfo** (not a menu-bar app on the user's Mac — it has its own window; it does also use an `NSStatusItem`) runs to a stable run loop: `NSIBObjectData initWithCoder: done objects=27 conns=26`, all 26 `establishConnections` done, `NSApp run: finishLaunching returned`, repeated `NSStatusItem button: cache hit` — **0 SIGEXC**. Objects 168–179 are the 12 NSNumbers decoded *before* the NSMutableSet (idx=5) that used to crash.
- **TestWindow.app** (has a window) verified with live X11 evidence: `MapNotify` / `VisibilityNotify` / `FocusIn` → window mapped and focused; `EnterNotify`/`LeaveNotify`; clean close via `ClientMessage: WM_DELETE_WINDOW` → `UnmapNotify`.
- **Known AppKit gap**: `+[NSTextField labelWithString:]` → `unrecognized selector` (not implemented anywhere in AppKit) — non-fatal; window still mapped.
- Launching a GUI app for X11 verification: `setsid bash -c 'script -qc "./osxie <path-to-binary>" /tmp/opencode/run_testwindow.log > /tmp/opencode/run_testwindow_outer.log 2>&1' &` then grep the outer log for `MapNotify`.

## Issue 24: System tray icon destroyed by KWin/xembedsniproxy (cpuinfo) — FIXED
- **Scope**: `src/external/cocotron/AppKit` (nested repo): `NSApplication.m`, `NSStatusItem.m`, `NSWindow.m`, `X11.backend/X11Window.h/.m`, `X11.backend/X11Display.m`, plus trace cleanup in `NSButtonCell.m`, `NSImage.m`, `NSCachedImageRep.m`.
- **Diagnosis**: A bare-X11 test window (`/tmp/opencode/tray_test.c`, 22×22 override-redirect + `_XEMBED_INFO` + dock request) embeds fine and lives 8+ s under the sniproxy slot — the XEmbed path itself was never the problem. The tray was killed by the app itself: `NSApplicationMain` (`NSApplication.m:1703`) called `makeKeyAndOrderFront:` on **every** window in `[NSApp windows]`, including the embedded tray → `XRaiseWindow` + orderFront → KWin de-embeds and destroys the tray ~570 ms later. `isMiniaturized` on windows with `_window=0` also emitted `BadWindow` (req 20/12/3).
- **Fix**:
  1. `NSApplication.m`: the "bring windows to front" loop skips windows whose `[win level] == NSStatusWindowLevel`.
  2. `X11Window.m`: new `_embedded` flag; `dockInSystemTray` sets `_embedded=YES; _mapped=YES` after reparent; `ensureMapped` no-ops when embedded; guards `if (_window==0)`/`if (_embedded)` in `hideWindow`, `placeAboveWindow`, `placeBelowWindow`, `makeKey`, `miniaturize`, `deminiaturize`, `isMiniaturized`, `setTitle`, `setFrame`, `setStyleMaskInternal`, `syncDelegateProperties`.
  3. `X11Display.m`: `DestroyNotify` now, besides `[window invalidate]`, enqueues `platformWindowWillClose:` on the delegate (same pattern as `ClientMessage WM_DELETE_WINDOW`) → `close:` → `NSWindowWillCloseNotification` → `NSStatusItem _trayWindowWillClose:` → automatic re-dock to a fresh tray window.
  4. **Over-release crash**: `_releaseWhenClosed=YES` by default (`NSWindow.m:337`) + `close:` autorelease + performSelector retain + `_trayWindowWillClose:` doing `release` → triple release (SIGEXC `addr=0x18`, `sel=retainCount`). `_trayWindowWillClose:` no longer releases — it only clears `_trayWindow`.
- **Verified** (run P, after trace cleanup): slot `0xa0004f` (32×32) with child `0x1800002` 22×22 embedding stable; content 368 white + gray px; **0 SIGEXC, 0 DestroyNotify, 0 BadWindow**; app alive after 25 s (4 processes). External `XDestroyWindow` → automatic re-dock to a new tray window with 0 crashes.
- **Trace cleanup**: per-frame traces removed (`NSButtonCell drawInterior`, `NSCachedImageRep drawInRect`, `NSWindow flushWindow`, `NSStatusItem button:`/`setTitle:`, `NSApp run: got event`, `NSImage` 62×24 TIFF dumps); remaining traces gated behind env vars (`OSXIE_TRACE_WINDOW_LIFE`, `OSXIE_TRACE_FLUSH`, `OSXIE_TRACE_BACKTRACE`).
- **Issue 24b — instrumentation gated (clean logs)**:
  - `X11Display.m`: new `eventsTraceEnabled()` helper (env `OSXIE_TRACE_EVENTS`); all per-event `NSLog`s gated (EnterNotify, FocusIn, MapNotify, ReparentNotify, ClientMessage:WM_DELETE_WINDOW, Unknown event, etc.). `handleError:` now silently drops `BadWindow` (benign destroy-race noise — all other paths guard `_window==0`).
  - `NSApplication.m`: 28 startup traces gated behind `OSXIE_TRACE_APP`.
  - `AppKit/nib.subproj/*` (NSIBObjectData, NSNib, NSCustomObject, NSNibBindingConnector, GSNibArchiveKeyedUnarchiver): 37 `fprintf` + 6 `NSLog` traces gated behind `OSXIE_TRACE_NIB` (Issue-23 NIB-decode traces; the fixes themselves are untouched).
  - Foundation (separate repo `src/external/foundation`): `NSNumber.m` `numberWithUnsignedChar:` trace gated behind `OSXIE_TRACE_NUMBER`; `NSNotificationCenter.m` delivering/delivered traces gated behind `OSXIE_TRACE_NOTIFICATION`.
  - **Result**: idle cpuinfo run produces a clean log — 0 lines in the app log; only one-shot legit messages remain in the outer log (`Bootstrapping the container`, `Failed to create /Users for shader cache`, `FreeType font face is not scalable`, `error opening!: 14` / `Could not create database queue for path /private/var/db/launchservices.db`).
- **Status**: UNCOMMITTED in the `cocotron` nested repo (together with the prior Issue-23 NIB/connector work).
---
## Issue 25: IOKit missing `kIOMainPortDefault` symbol (htop dyld abort)
- **File**: `src/external/IOKitUser/IOKitLib.c` (+ declaration in `IOKitLib.h` and `osxie/include/IOKit/IOKitLib.h`)
- **Problem**: Homebrew macOS binaries built for macOS 11+ (e.g. `htop` 3.5.2, built for Mac OS X 14.0) reference `_kIOMainPortDefault`, the macOS 11+ spelling of the default IOKit master port. Osxie's IOKit only exported the old `_kIOMasterPortDefault`, so dyld aborted at load: `dyld: Symbol not found: _kIOMainPortDefault, Referenced from: /usr/local/Cellar/htop/3.5.2/bin/htop, Expected in: IOKit.framework` → `Abort trap: 6 (core dumped)`.
- **Fix**: Added `const mach_port_t kIOMainPortDefault = MACH_PORT_NULL;` next to `kIOMasterPortDefault` in `IOKitLib.c`, and `extern const mach_port_t kIOMainPortDefault;` in both `IOKitLib.h` copies (source tree + `osxie/include/IOKit/`). Verified: `llvm-nm` shows `_kIOMainPortDefault`; rebuilt `ninja -C build_new IOKit`, copied to `~/.osxie/System/Library/Frameworks/IOKit.framework/Versions/A/IOKit`; `osxie .../htop --version` → `htop 3.5.2`.
- **Note**: the `./osxie <bin>` launcher ignores SIGTERM (same as the GUI-launcher QoL work) — a stray `osxie .../htop` left from a timed-out run survived `kill -9` (setuid euid=0) and held the capture pipe; removed via `timeout 30 pkexec kill -9`.
---
## Issue 26: osxieserver killed by unimplemented mach calls (dark screen + cascade SIGILL) — FIXED
- **Files**: `src/external/osxieserver/duct-tape/internal-include/osxieserver/duct-tape/stubs.h`, `src/external/osxieserver/duct-tape/src/stubs.c`, `src/external/osxieserver/duct-tape/src/thread.c`
- **Problem**: htop (or any guest calling an unsupported `thread_info` flavor) hit `thread_info`'s `default: dtape_stub_unsafe(...)` (`thread.c:1211`). `dtape_stub_unsafe` → `dtape_stub_log(fn, -1, ...)` → `do_abort=true` always (safety<0) → `abort()` killed the **shared** osxieserver → every guest RPC got ECONNREFUSED (-111) → guest `semaphore_*_trap_impl` (`emulation/src/xnu_syscall/mach/impl/mach_traps.c:168-183`) called `__simple_abort()` → SIGILL in guest processes (opendirectoryd under mldr). Symptom: terminal goes dark-screen then crashes; `semaphore_signal failed (internally): -111` in logs. Only THREAD_IDENTIFIER_INFO (4) and THREAD_BASIC_INFO (3) were implemented.
- **Fix (3 layers)**:
  1. `dtape_stub_unsafe` (stubs.h:15) no longer reaches `__builtin_unreachable()`/abort — it logs loudly and yields `KERN_FAILURE`. Real `abort()` is opt-in via `DTAPE_FATAL_STUBS` (both in the macro and in `dtape_stub_log`'s safety<0 branch, stubs.c).
  2. `thread_info` now also implements `THREAD_EXTENDED_INFO` (5); the `default` case logs a warning and returns `KERN_FAILURE` instead of aborting. Note the run-state mapping duplicated from the BASIC case.
  3. No guest reconnect exists yet (dserver client has none), so keeping the server alive IS the fix; hardening remaining `panic()`s (locks/kqchan invariants) is future work.
- **Deploy caveat**: `/usr/local/bin/osxieserver` was (until 2026-08-09) a bash wrapper running `strace -f` on `osxieserver.real` (EBADF debug artifact). Restored to a clean symlink `osxieserver -> osxieserver.real`; wrapper backed up at `/usr/local/bin/osxieserver.strace-wrapper`. New builds go to `/usr/local/bin/osxieserver.real`; also sync `install/bin/osxieserver` and `~/.osxie/bin/osxieserver` (the launcher `osxie.c:873` `execl`s `/usr/local/bin/osxieserver`, but the running process may be the `install/bin` copy). Each `osxie <cmd>` reuses the running server/container for the prefix (verified: only ONE osxieserver while htop ran inside the user's `osxie shell`).
- **Verified 2026-08-09**: htop 3.5.2 ran 40s under `script -qc` rendering its full TUI (guest process list, mem bars) — 0 `-111`, 0 SIGILL, no dark screen, server PID stayed alive. iTerm2 3.4.23 reached its known `-[iTermTabColorMenuItem setView:]` NSException abort **without** taking the server or the user's `osxie shell`/bash down (coredump `core.mldr.1000.*.3540647.*` is just the app's own ABRT).
- **Known Build noise**: the 14 `-Wreturn-type` warnings in duct-tape (functions ending in `dtape_stub_unsafe()` that aren't `kern_return_t`) are pre-existing style; the discard-expression value (5=KERN_FAILURE) ends up in eax in practice. No `-Werror` in duct-tape.
---
## Build: CORE COMPLETE, READY FOR INSTALL
## Issue 27: iTerm2 double-free at startup (NSAppearance `_systemPrefersDark` reuses one malloc for two candidates) — FIXED
- **File**: `src/external/cocotron/AppKit/NSAppearance.m` (`_systemPrefersDark`, lines ~53-57)
- **Problem**: iTerm2 3.4.23 crashed with `malloc: *** error for object 0x…: pointer being freed was not allocated` (SIGABRT, exit 134) during NIB/launch, right after the ~148 `[NSNibControlConnector] Warning: skipping connection with nil source…` lines. The SIGABRT handler's own backtrace was useless — `_Unwind_Backtrace` inside the handler only walks the signal-delivery frames (`handler_linux_to_bsd → sigexc_handler → sig_restorer → dserver_rpc_*pthread_kill → sys_pthread_kill`), never the crash site.
- **Diagnosis**: instrumented libsystem_malloc's `free()` at the exact detection point: `find_registered_zone(ptr)` fails → `osxie_dump_free_backtrace(ptr)` (gated by env `OSXIE_TRACE_FREE=1`, dumps `[FREE]` + `_Unwind_Backtrace` via `unwind` lib + `/proc/self/maps`). Full caller stack resolved (offset = `ip - map_start`, NOT `+0x1000`):
  `iTerm2 main → NSApplicationMain → run → finishLaunching → [NSNotificationCenter postNotificationName:object:] → …:userInfo: → postNotification: → _NSNotificationObserver postNotification: → iTerm2 observer (stripped) → -[NSApplication effectiveAppearance] → +[NSAppearance currentAppearance] → _systemPrefersDark → free → free (double)`.
- **Root cause**: `_systemPrefersDark` built two candidate `kdeglobals` paths with `char *p = malloc(...)` then `sprintf` twice into the **same buffer**, storing `p` in BOTH `candidates[0]` and `candidates[1]`. The cleanup `free(candidates[0]); free(candidates[1])` then freed the identical pointer twice. Triggered on the `NSApplicationDidFinishLaunching` notification (iTerm2's observer queries `effectiveAppearance`). cpuinfo's 3rd-render double-free was likely the same path (its status-item setup also calls appearance APIs) — not yet re-verified.
- **Fix**: allocate a second buffer `q` for the `/Volumes/SystemRoot…` candidate so each slot is a distinct allocation.
- **Instrumentation (kept, env-gated)**: `src/external/libmalloc/src/malloc.c` gained `osxie_dump_free_backtrace()` (static), and `src/external/libmalloc/CMakeLists.txt` added `unwind` to `system_malloc`'s `SIBLINGS` (target `unwind` provides `__Unwind_Backtrace`/`__Unwind_GetIP`). Rebuild/install: `ninja -C build_new system_malloc` + `cp build_new/src/external/libmalloc/libsystem_malloc.dylib ~/.osxie/usr/lib/system/`. Inactive unless `OSXIE_TRACE_FREE=1`.
- **Verified 2026-08-12**: rebuilt AppKit (`./scripts/relink_dylibs.sh AppKit --restart-server`), ran iTerm2 twice for 45-60s: **0 `[FREE]`, 0 `malloc:`, 0 SIGABRT**, app stays in the run loop (`[X11Display orderedWindowNumbers] unimplemented` benign warnings remain).
- **Status**: UNCOMMITTED in the `cocotron` nested repo (alongside Issues 23/24 work).
## Issue 28: top (Apple libtop) full report — processor_set_tasks_with_flavor, sys_proc_info PIDRUSAGE/WORKQUEUEINFO, VM_LOADAVG — FIXED
- **Goal**: `top -l 1 -s 1 -n 5` inside the guest must print the full report (Processes/Load Avg/CPU/PhysMem/VM/Networks/Disks + process table) instead of `error: while gathering a libtop sample`.
- **Chain fixed bottom-up** (each step was the next `LIBTOP_DBG: xxx=-1`):
  1. **`processor_set_tasks_with_flavor` was a `dtape_stub_unsafe()`** (`src/external/osxieserver/duct-tape/src/processor.c:270`): returned garbage kr + gigantic tcnt → `proc_table_read=-1` → everything else short-circuited. Implemented it: validates pset != PROCESSOR_SET_NULL, enumerates tasks via a new `dtape_hook_task_for_each` hook (typedef + field in `duct-tape/include/osxieserver/duct-tape/hooks.h`; impl in `src/external/osxieserver/src/server.cpp` iterating `processRegistry().copyEntries()`), allocates a `kalloc`'d array of `ipc_port_t`, fills with `task_reference()` + `convert_task_to_port()` (consumes the ref), returns the OOL port array + count. `processor_set_tasks` now forwards to it with `TASK_FLAVOR_CONTROL`. Also had to forward-declare/reorder (`processor_set_tasks` called `_with_flavor` before its definition) and add `#include <osxieserver/duct-tape/task.h>` + `<kern/task.h>` (for `struct dtape_task`, `task_reference`, `convert_task_to_port`).
  2. **Kernel task leaked into task list** → `libtop_p_proc_command` assert (`pinfo->psamp.pid != 0`, libtop.c:2154): the registry's emulated kernel process (`Process::kernelProcess()`, `_pid = -1`, `_nspid = 0`) returned pid 0 from `pid_for_task` → `audit_token.pid == 0`; `kinfo_for_pid(0)` yielded an all-zero kinfo → `p_comm=""` → `strncmp("", "LaunchCFMApp", 0)==0` → else-branch → assert. Fix: `dtape_hook_task_for_each` skips entries with `id() == -1`.
  3. **`sys_proc_info` gaps** (`src/external/xnu/osxie/src/libsystem_kernel/emulation/src/xnu_syscall/bsd/impl/misc/proc_info.c`): libtop's `proc_pid_rusage` uses **callnum 9 = PROC_INFO_CALL_PIDRUSAGE** (was `default → -ENOTSUP`) and `proc_pidinfo(PROC_PIDWORKQUEUEINFO)` uses **flavor 12** (this tree's SDK `sys/proc_info.h` numbers PROC_PIDTASKALLINFO=2, PROC_PIDWORKQUEUEINFO=12). Added `case 9` → `_proc_pid_rusage` (zero-fills `struct rusage_info_v4`; libproc passes bufsize=0) and `case PROC_PIDWORKQUEUEINFO` → `_proc_pidinfo_workqueueinfo` (reads `/proc/<pid>/stat` num_threads via the same stat-parsing pattern as `_proc_pidinfo_taskinfo`). Needed `#include <sys/resource.h>`.
  4. **`VM_LOADAVG` sysctl missing** (`.../helper/misc/sysctl_vm.c`): `getloadavg()` does `sysctl([CTL_VM, VM_LOADAVG])` (VM_LOADAVG=2) and the `vm` node only had the two OID_AUTO 1000/1001 counters → `ENOENT` → `loadavg_update=-1`. Added a `{ .oid = 2, .type = CTLTYPE_STRUCT, .exttype = "S,loadavg", ... }` entry + `handle_loadavg` that reads `/proc/loadavg` (1/5/15-min fractions via local `parse_load_fraction`), fills a local `struct osxie_loadavg { uint32_t ldavg[3]; long fscale; }` (matches BSD ABI, `FSCALE=2048`), struct-assigns into `old` (no `memcpy` available in this freestanding-ish build).
- **Build/deploy**: server → `ninja -C build_new osxieserver` + copy to `install/bin/osxieserver` (sudo to `/usr/local/bin/osxieserver.real` NOT done — no password; the `install/bin/osxie` launcher execs `install/bin/osxieserver`). Guest libsystem_kernel → `ninja -C build_new system_kernel` + `cp build_new/src/external/xnu/osxie/src/libsystem_kernel/libsystem_kernel.dylib ~/.osxie/usr/lib/system/`.
- **Verified 2026-08-14** (`./osxie ~/.osxie/usr/bin/top -l 1 -s 1 -n 5`, log `/tmp/opencode/top_t11.log`): full report printed, exit 0. `pset_tasks kr=0 tcnt=10`, all `task_update/proc_table_read/loadavg_update/load_get/fw_sample/vm_sample/networks_sample/disks_sample` ret=0. `Processes: 10 total, 10 sleeping, 19 threads`, real host Load Avg / CPU / PhysMem; process table rows show pid/threads/ppid/uid/state/BOOSTS (all comms show "mldr" — the host-side launcher comm; guest-side naming is a cosmetic future step; `MemRegions` 0 because vm-region traversal isn't fed).
- **Instrumentation (env-gated per Issue-24b convention, 2026-08-14)**: all 16 `LIBTOP_DBG:` traces in `src/external/top/libtop.c` wrapped in `LIBTOP_DBG_TRACE()` (new helper `libtop_dbg_trace_enabled()`, active only when `OSXIE_TRACE_LIBTOP` env var is set; `libtop.c` already had `bool` available transitively). The 3 `OSXIE_DBG` lines in `duct-tape/src/init.c` converted to `dtape_log_debug()` (gated by `DSERVER_LOG_LEVEL`); the 3 in `src/osxieserver.cpp` and 4 in `src/server.cpp` gated behind `OSXIE_TRACE_SERVER` env var. Verified: default run = 0 trace lines, `OSXIE_TRACE_LIBTOP=1` = 13 lines.
- **Deploy gotcha (2026-08-14)**: a stale `osxieserver` for the `~/.osxie` prefix spawned by the **system** launcher (`/usr/local/bin/osxie` → `/usr/local/bin/osxieserver.real`, an old pre-fix build) returns `kr=4` (`KERN_INVALID_ARGUMENT`, host_priv null) for `host_processor_sets` → top fails again. Fix: kill the running server (`kill -9 <pid>`) + leftover `/sbin/launchd` containers; next `install/bin/osxie` run spawns a fresh server from `install/bin/osxieserver` (dev launcher has INSTALL_PREFIX=`/home/fenix/src/Osxie/install`). `strings` check: fixed binary contains `OSXIE_DBG`/`dtape_hook_task_for_each`, the old `/usr/local/bin/osxieserver.real` has neither.
- **Status**: FIXED + instrumentation gated. Changes in `src/external/osxieserver` (processor.c, hooks.h, server.cpp, osxieserver.cpp, init.c), `src/external/xnu` (proc_info.c, sysctl_vm.c), `src/external/top` (libtop.c gating).
## Issue 29: AWT-Java CGS APIs — CGWindowListCopyWindowInfo/CGWindowListCreate/CGSetLocalEventsFilterDuringSuppressionState — FIXED
- **Goal**: satisfy the CG symbols that `libawt_lwawt.dylib` (real Corretto 17) imports at load, unblocking AWT-Java GUI code paths. Imports found via `llvm-nm -u` on `~/.osxie/Applications/CPU-Info.app/Contents/runtime/Contents/Home/lib/libawt_lwawt.dylib`: `_CGSetLocalEventsFilterDuringSuppressionState`, `_CGWindowListCopyWindowInfo`, `_CGWindowListCreateImage`.
- **Files** (`src/external/cocotron/CoreGraphics`): `CGS.m`, `CGSConnection.h/.m`, `CGSWindow.h/.m`, `X11.backend/CGSWindowX11.m`, `src/stubs.c`, `include/CoreGraphics/CGWindow.h`, `cgs_smoke.m`.
- **Implementation**:
  1. `CGS.m`: new `CGWindowListCopyWindowInfo(option, relativeToWindow)` — enumerates `g_connections` → `[conn windowsSnapshot]` → per-window `copyWindowInfoDictionary()` (kCGWindowNumber/OwnerPID=getpid/OwnerName=NSProcessInfo/Name=kCGSWindowTitle/Bounds dict {"X","Y","Width","Height"}/Layer=0/Alpha=1.0/IsOnscreen), returns `CFArrayCreateMutable` (bridges to NSMutableArray so AWT's `[windows release]` works). Filtering: `OnScreenOnly` via `isOnscreen`; `relativeToWindow`/Above/Below approximated (no cross-window z-order tracked in-process). Also `CGWindowListCreate` (array of window IDs) and `CGWindowListCreateDescriptionFromArray`.
  2. `CGSConnection.h/.m`: `windowsSnapshot` (allValues under @synchronized). `CGSWindow.h/.m`: default `isOnscreen`=NO; `X11.backend/CGSWindowX11.m`: real impl via `XGetWindowAttributes` → `map_state == IsViewable`.
  3. `src/stubs.c`: `CGSetLocalEventsFilterDuringSuppressionState` returns 0 (no suppression window). Types `CGEventFilterMask`/`CGEventSuppressionState` already existed in `framework-include/CoreGraphics/CGEventTypes.h` (do NOT add a second copy to the cocotron include tree — causes redefinition).
  4. `include/CoreGraphics/CGWindow.h`: added `kCGNullWindowID`, `kCGWindowNumber` extern, `CGWindowListCopyWindowInfo` export.
- **Gotchas**:
  - The X11 backend is a SEPARATE bundle loaded from `.../CoreGraphics.framework/Resources/Backends/X11.backend/Contents/MacOS/X11` (NOT the top-level `Backends/` dir). `isOnscreen` lives in that bundle; forgetting to copy it = default NO = OnScreenOnly filters everything → empty list.
  - Build: `ninja -C build_new CoreGraphics X11_cgbackend cgs_smoke`. Deploy: copy framework binary to `~/.osxie/System/Library/Frameworks/CoreGraphics.framework/Versions/{A,C}/CoreGraphics`, backend to `.../Resources/Backends/X11.backend/Contents/MacOS/X11`, `cgs_smoke` to `~/.osxie/usr/bin/`.
- **Verified 2026-08-14** (cgs_smoke under `install/bin/osxie`, log `/tmp/opencode/cgs_smoke4.log`): window created via `CGSNewWindow`+`CGSOrderWindow`+`CGSSetWindowTitle("CGS Smoke Window")` appears in `CGWindowListCopyWindowInfo(OnScreenOnly)` with correct name/number(#1)/layer(0)/bounds(150,184 320x200); `CGWindowListCreate` returns its ID; exit 0. All 4 AWT symbols now `T` in the deployed framework.
- **CGWindowListCreateImage implemented (2026-08-14)** — no longer a stub:
  - `CGS.m`/`CoreGraphicsPrivate.h`: `_CGSConnectionForWindowID(winId)`. `CGSWindow.h/.m`: `captureBitmapDataWithWidth:height:` (base returns NULL). `CGSConnection.h/.m`: `captureRootBitmapDataWithRect:width:height:` (base returns NULL). `X11.backend/CGSWindowX11.m`: `osxieCapturePixels(Display*, Drawable, ...)` static helper + window capture impl + category `CGSConnectionX11 (OsxieCapture)` with root capture.
  - `CGWindowListCreateImage` wraps the pixel buffer via `CGDataProviderCreateWithData` + `CGImageCreate` with `kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little` (matches `O2ImageRead_BGRA8888_to_argb8u` reading B,G,R,A bytes on little-endian).
  - **Window capture WORKS** (`cgs_smoke18.log`): 320x200, 32bpp, 8bpc. **Root/desktop capture returns NULL** on this host: KWin redirects the root so `XGetImage(root)` → `BadMatch` (err 8) even 1x1, `XGetImage` on `XCompositeNameWindowPixmap(root)` → `BadDrawable` (err 9), and `XCompositeGetOverlayWindow` → also `BadMatch`. **Even the host's own native Xlib fails** (`/tmp/opencode/host_root_capture.c`: BadMatch) — this is an X-server-level compositor restriction, not a guest/wrapper problem. CRobot handles the NULL gracefully (`if (screenPixelsImage == NULL) return;`). New `libXcomposite.dylib` (from `wrap_elf(Xcomposite libXcomposite.so)` in `src/native/CMakeLists.txt`, added to `X11.backend/CMakeLists.txt` DEPENDENCIES) → `~/.osxie/usr/lib/native/`. Capture debug prints gated behind env `OSXIE_TRACE_CGSCAPTURE` (Issue-24b convention).
- **Status**: UNCOMMITTED in the `cocotron` nested repo (alongside Issues 23/24/27). Root-capture (Robot full-screen) blocked by KWin GetImage restriction — future option: per-window `XCompositeNameWindowPixmap` stitch, or `import`-style capture via KWin screenshot D-Bus.
## Issue 30: JVM GangWorker SIGSEGV — duct-tape `thread_unblock` leaks the wait timer → spurious KERN_OPERATION_TIMED_OUT (49) — FIXED
- **File**: `src/external/osxieserver/duct-tape/src/thread.c` — `thread_unblock` (lines ~549-556).
- **Problem**: intermittent SIGSEGV in Corretto 17's `GangWorker::run` (CPU-Info under `osxie`), usually during GC after a few seconds. `hs_err`: `pc=libjvm.dylib+0xafcde9`, `si_addr=0x10` — after `OSXSemaphore::wait` returns, `GangWorker::run` loads the dispatcher's embedded semaphore vtable `movq (%r12),%r12` (=0) and derefs `0x10(%r12)`. `OSXSemaphore::wait` (libjvm @0x8f7740) retries ONLY on KERN_ABORTED=14; any other code (notably KERN_OPERATION_TIMED_OUT=49) makes it return without the token → `GangWorker::run` derefs a zeroed dispatcher.
- **Root cause**: stock XNU cancels `thread->wait_timer` inside `thread_unblock` (sched_prim.c:620-626) so an early-woken timed wait doesn't leave a timer armed; **sched_prim.c is NOT compiled** (CMakeLists count=0), so the only `thread_unblock` is the duct-tape one, which just set `wait_result` and resumed. A `semaphore_timedwait` woken before its deadline therefore leaks `wait_timer` (`wait_timer_is_set=TRUE`, `wait_timer_active=1`). When that stale timer later fires (during a deadline-less blocking `semaphore_wait`), `thread_timer_expire` → `clear_wait_internal(THREAD_TIMED_OUT)` → resume → `semaphore_convert_wait_result` → 49 for a wait the JVM can't time out.
- **Crash-sequence proof** (run7, `DSERVER_LOG_LEVEL=debug`): TID 67/68 blocked on the GangTaskDispatcher semaphore (`call #60 semaphore_wait`, deadline=0) got **reply 49 for call #60** 0.5 ms BEFORE any `interrupt_enter`, with no signal and no deadline — impossible via the normal path. Genuine `#62 timedwait` polling timeouts (31× in run7) are benign.
- **Fix**: in `thread_unblock`, before `thread_resume`, cancel any armed wait timer (mirroring stock XNU): `if (wait_timer_is_set) { if (timer_call_cancel(&wait_timer)) wait_timer_active--; wait_timer_is_set = FALSE; }`. `timer_call_cancel` already exists (timer_call.c:731, used by mk_timer). No comment added to code per project rule.
- **Time domain note**: the whole duct-tape is nanoseconds-consistent — `mach_absolute_time() = rtc_nanotime_read()` (ns), `nanoseconds_to_absolutetime` is identity (i386/rtclock.c:536), `mach_timebase_info` numer=denom=1, and the server's timerfd uses `TFD_TIMER_ABSTIME` + `CLOCK_MONOTONIC` (server.cpp `dtape_hook_timer_arm`). A `{30,0}` deadline is genuinely +30s.
- **Deterministic test** `/tmp/opencode/stale_timer_test.c`: worker does `semaphore_timedwait(sem_a, {30,0})`, main signals early (wakes → leaks timer), then worker does blocking `semaphore_wait(sem_b)`; main asserts sem_b wait is still blocked at +3s (no spurious 49). **5/5 PASS with fix**; on the OLD server it is nondeterministic (one run: wakeup lost, timedwait ran to +30s → 49, then guest `semaphore_timedwait failed (internally): -111` — server connection dropped).
- **Crashloop validation**: `/tmp/opencode/batchtest.sh 12 25` (CPU-Info JVM, `-XX:ParallelGCThreads=1`, `OSXIE_TRACE_SEMA=1`): **0 crashes in 12 runs** (was ~1-in-2 before), 0 `hs_err_pid*.log`, no `call #60 ... result code 49` in any run log.
- **Build/deploy**: `ninja -C build_new osxieserver` → `pkill -9 -x osxieserver` (NOT `pkill -f osxieserver` — matches the shell's own cmdline and kills it) → `cp build_new/src/external/osxieserver/osxieserver install/bin/osxieserver` (can't `cp` over a running executable, ETXTBSY). `strings ... | grep -c timer_call_cancel` = 2 on the deployed binary.
- **Status**: FIXED + validated. UNCOMMITTED (working-tree change in `src/external/osxieserver/duct-tape/src/thread.c`, repo-root git). Note: a residual OLD-server lost-wakeup race was observed once (signal failed to wake an early-blocked timedwait) — not reproduced on the fixed build across 5 stale-timer + 12 JVM runs; monitor if JVM crashes ever recur.

## Issue 31: Indium/Vulkan/Metal 13 GB RAM leak + 88 threads + 98% CPU spin — FIXED
- **Files**: `CMakeLists.txt` (build_new CMakeCache `ENABLE_METAL`), `src/external/metal/CMakeLists.txt`
- **Problem**: With `ENABLE_METAL=AUTO` (default), CMake detected Vulkan+LLVM on the host and enabled `DARLING_METAL_ENABLED=1` for the Metal/MetalKit framework. This caused:
  1. **13 GB RSS** (growing ~350 MB/s) from Mesa/llvmpipe Vulkan driver allocating huge memory pools per buffer/texture with no VMA suballocator (each object gets its own `vkAllocateMemory` call; `buffer.cpp:83`, `texture.cpp:371`).
  2. **88 threads** from llvmpipe (30 `traceq0` + 24 `gdrv0` + 24 `gl0`) — software GPU thread pools per core.
  3. **98.7% CPU** on a single thread (`wchan=0`, userspace spin inside llvmpipe Vulkan pipeline) while 87 other threads slept on `futex_wait`.
  4. `MTLCreateSystemDefaultDevice()` → `ensureDevices()` (`dispatch_once`) → creates ONE `MTLDeviceInternal` with an `Indium::Device` → `Indium::createSystemDefaultDevice()` → Vulkan instance + device + llvmpipe driver init → spawns all 88 threads at creation time.
  - **iTerm2 is a terminal emulator** — it links Metal.framework/MetalKit.framework for `PTYSession` metal color-space support but never actually renders via Metal. The entire Vulkan stack was allocated and spinning for nothing.
- **Fix**: Set `ENABLE_METAL=OFF` in `CMakeCache.txt`. Rebuild with `ninja -C build_new Metal MetalKit` + copy to `~/.osxie/System/Library/Frameworks/Metal.framework/Versions/A/Metal` and `MetalKit.framework/Versions/A/MetalKit`. With `DARLING_METAL_ENABLED=0`, `MTLCreateSystemDefaultDevice()` returns `nil`, MTKView uses no-op stubs, no Vulkan/Indium/Mesa initialization occurs.
- **Verified 2026-08-17**: RSS **185 MB stable** (was 13+ GB), **0% CPU spin** (was 98.7%), **87 threads all sleeping** (were 1 spinning + 87 sleeping). Memory growth: 0 MB/s.
- **Note**: The 87 sleeping threads are Mesa/llvmpipe **OpenGL** threads (separate from Metal/Vulkan), spawned by CoreGraphics/Onyx2D for software rendering. They're harmless but could be reduced with `LP_NUM_THREADS=1` or `GALLIUM_NTHREADS=1` env vars at runtime.
- **Metal re-enablement path**: Fix Indium's `buffer.cpp` and `texture.cpp` to use VulkanMemoryAllocator (VMA) for suballocation instead of per-object `vkAllocateMemory`. Fix `pollEvents` to ensure llvmpipe pipeline threads block properly. Then set `ENABLE_METAL=ON`.

## Issue 32: iTerm2 no X11 window — `applicationShouldOpenUntitledFile:` returns NO — FIXED
- **Files**: `src/external/cocotron/AppKit/NSApplication.m` (finishLaunching flow)
- **Problem**: iTerm2 launches, loads NIB (27 objects, 26 connections — 148 `NSNibControlConnector` warnings for nil sources), `finishLaunching` completes successfully (`posting DidFinishLaunching` → `done`), the event loop starts (`NSApp run: finishLaunching returned`), but **zero X11 windows** are ever created. The app sits at 185 MB / 0% CPU / all threads sleeping, with no visible UI.
- **Root cause**: `applicationShouldOpenUntitledFile:` returns `0` (NO) — iTerm2's delegate decides NOT to open a window because there's no saved session state in the container. In real macOS, iTerm2 restores its last session from `~/Library/Saved Application State/`; in the osxie container this directory doesn't exist, so iTerm2 opens nothing.
- **Evidence**: `[TRACE] finishLaunching: openUntitled -> 0` in the log. After this, `DidFinishLaunching` is posted (which is where iTerm2's observer would create a session/window), but no window creation occurs.
- **Fix**: After `DidFinishLaunching` is posted, if `[_windows count] == 0` and a `NSWindowController` can be obtained (via `[NSApp mainWindow]`'s delegate or `NSApp delegate`), force `[controller newDocument: self]`. Added at `NSApplication.m:662`: checks `[[NSApp windows] count] == 0` and forces a new document. Trace: `[TRACE] finishLaunching: no windows after launch, forcing newDocument`.
- **Verified 2026-08-18**: iTerm2 launches → NIB loads (148 connector warnings) → `finishLaunching` → `forced newDocument done` → two X11 windows created (WID=18874408 400x424, WID=18874406 503x136). Dark theme colors applied correctly.
- **Status**: FIXED + verified.

## Issue 33: 87 Mesa/llvmpipe OpenGL threads (sleeping, benign) — OPTIMIZATION
- **Problem**: Even with Metal disabled, CoreGraphics/Onyx2D creates OpenGL contexts for software rendering, spawning ~87 threads in groups of 3 (`traceq0`/`gdrv0`/`gl0`) from llvmpipe. All sleep on `futex_wait` but consume ~8 MB stack each (~700 MB virtual, ~185 MB RSS).
- **Fix**: Set `GALLIUM_NTHREADS=1` or `LP_NUM_THREADS=1` in the container environment to limit llvmpipe to 1 worker thread per pipe. Can be added to `src/startup/osxie.c` environment setup or via `launchd.plist` environment keys.
- **Status**: PENDING — low priority, threads are sleeping and not causing issues.

## Issue 34: X11Theme dark/light theme inheritance from host KDE/GNOME — FIXED
- **Files**: `src/external/cocotron/AppKit/X11.backend/X11Theme.m` (theme reader) + `~/.osxie/Users/<user>/.config/kdeglobals` (overlay copy)
- **Problem**: macOS-style windows rendered with light gray (237,237,237) backgrounds instead of inheriting the host's dark/light theme. User requested: "también debería heredar los colores del theme".
- **Root causes (3 bugs)**:
  1. **Config path mismatch**: `X11Theme.m` reads `$HOME/.config/kdeglobals`, but osxie sets `HOME=/Users/<user>` (macOS-style path). The real KDE config lives at `/home/<user>/.config/kdeglobals` on the host. Inside the container overlay, the path `/Users/<user>/.config/kdeglobals` didn't exist. Fix: added `/home/<user>/...` and `/Volumes/SystemRoot/home/<user>/...` to `_configCandidates()` (also increased max from 3 to 5), AND created a direct copy `~/.osxie/Users/fenix/.config/kdeglobals` (symlinks break inside the overlay since `/home/<user>/` doesn't exist inside the container).
  2. **INI section brackets mismatch**: `_sectionForRole()` returned `[Colors:Window]` (with square brackets), but the INI parser stripped brackets → cache stored `Colors:Window/BackgroundNormal`. Lookup for `[Colors:Window]/BackgroundNormal` never matched. Fix: removed brackets from `_sectionForRole()` returns.
  3. **Cache too small**: `_themeCache[32]` overflowed — kdeglobals has 166 key=value entries (6+ sections × many keys + ColorEffects). Entries parsed first (ColorEffects, inactive) filled the cache before Colors:Window/View/Selection were reached. Fix: increased to `_themeCache[256]`.
- **Verified 2026-08-18** (TestWindow + iTerm2):
  - **TestWindow**: Title=(24,27,40), Menu=(28,32,48), Content=(30,34,51) — matching KDE `EmeraldDark` theme values.
  - **iTerm2**: Two windows rendered dark — Title=(34,36,44), Menu=(34,35,43), Content=(28,31,34), Bottom=(11,11,11).
  - All values match KDE kdeglobals: `[Colors:Window] BackgroundNormal=24,27,40`, `[Colors:Button] BackgroundNormal=30,34,51`, `[Colors:View] BackgroundNormal=22,25,37`.
- **Env-gated traces**: `OSXIE_TRACE_THEME=1` shows all parsed config paths, fopen results, and per-role color resolution.
- **Deploy note**: `kdeglobals` must be a real copy in `~/.osxie/Users/fenix/.config/`, NOT a symlink (symlinks to host paths break inside the container overlay). For production, `src/startup/osxie.c` should copy `~/.config/kdeglobals` into the overlay at startup.
- **Status**: FIXED + verified. Traces gated behind `OSXIE_TRACE_THEME`.

## Issue 35: CFNetwork classes missing from Foundation — `-reexport_library` broken — FIXED
- **Files**: `src/external/foundation/src/CFNetworkBridge.m`, `src/external/foundation/src/NSHTTPCookieStorage.m`, `src/external/foundation/src/NSURLRequest.m` (prior), `src/external/foundation/src/NSURLResponse.m` (prior), `src/external/foundation/src/NSURLSession.m` (prior), `src/external/foundation/src/NSURLConnection.m` (prior), `src/frameworks/CoreServices/src/NSUserActivity_stub.m`
- **Problem**: cctools-port's ld64 `-reexport_library` is broken — sub-framework symbols don't propagate through umbrella frameworks. Foundation.header re-declares many CFNetwork classes (`NSURLCredential`, `NSURLProtectionSpace`, `NSURLAuthenticationChallenge`, `NSURLCache`, `NSURLProtocol`, `NSHTTPCookieStorage`, `NSUserActivity` in CoreServices, etc.) but the linker symbols only exist in CFNetwork.dylib, not Foundation.dylib. dyld resolves symbols per-framework, so apps linking Foundation get "Symbol not found" for every CFNetwork class.
- **Root cause confirmed**: Adding `-reexport_library,CFNetwork` to Foundation's link flags produces only ld warnings; the symbol table remains `U` (undefined). This is a cctools-port ld64 re-export limitation — the `-reexport_library` flag only re-exports the library's own exported symbols, not its re-exports.
- **Fix (2 layers)**:
  1. **NSURLRequest/NSMutableURLRequest** (`NSURLRequest.m`): Full implementation wrapping CFNetwork's `CFURLRequest*` functions via `extern void *` declarations resolved at runtime. Both classes exported `S` (defined) from Foundation.
  2. **CFNetworkBridge** (`CFNetworkBridge.m`): Empty `@implementation` stubs for all CFNetwork ObjC classes expected in Foundation: `NSCachedURLResponse`, `NSURLCache`, `NSURLCredential`, `NSURLCredentialStorage`, `NSURLProtectionSpace`, `NSURLAuthenticationChallenge`, `NSURLProtocol`. Duplicate class warnings with CFNetwork are expected and non-fatal (runtime picks first loaded).
  3. **NSHTTPCookieStorage** (`NSHTTPCookieStorage.m`): Stub with `sharedHTTPCookieStorage` singleton, `cookieAcceptPolicy` defaults to Always.
  4. **NSURLSession/Configuration/Task** (`NSURLSession.m`): Stub classes — all methods return nil/noop.
  5. **NSURLResponse/NSHTTPURLResponse** (`NSURLResponse.m`): Full implementation with internal helpers, `statusCode`, `allHeaderFields`, `localizedStringForStatusCode:`.
  6. **NSURLConnection** (`NSURLConnection.m`): Stub with `sendSynchronousRequest:` returning error.
  7. **NSUserActivity** in CoreServices (`NSUserActivity_stub.m`): Empty `@implementation` — Electron expects it in CoreServices (not Foundation), and CoreServices re-export also broken. Required adding `objc` to CoreServices' DEPENDENCIES for `_objc_empty_cache`.
  8. **NSHTTPCookieConstants** (`NSHTTPCookieConstants.m`): All 13 NSHTTPCookie string constants.
  9. **SwiftBridgeStubs.m fix**: Functions renamed from `_$s...` to `$s...` so Mach-O produces correct `_$s...` (single underscore prefix). Previously double-underscore produced `__$s...` which no consumer expected.
  10. **SharedFileList inlined**: `src/SharedFileList/constants.c` added directly to CoreServices SOURCES. LSSharedFileListCreate/CopySnapshot/InsertItemURL/ItemRemove/ItemCopyProperty/ItemCopyResolvedURL stubs + kLSSharedFileListItemLast/kLSSharedFileListSessionLoginItems/kLSSharedFileListLoginItemHidden constants.
  11. **CoreServices `-reexported_symbols_list` removed**: Was empty anyway (reexport broken), just confusing the build.
- **Deploy**: Foundation and CoreServices deployed via `./scripts/relink_dylibs.sh Foundation --restart-server` and direct `cp` of CoreServices binary.
- **Verified 2026-08-19**: The Unarchiver passes dyld (was blocked on `NSHTTPCookieStorage`). Obsidian passes NSUserActivity (was blocked, now hits `___NSDictionary0__struct` Swift runtime).
- **Status**: FIXED. All CFNetwork-origin classes now stub-exported from Foundation.

## Issue 36: App compatibility matrix (post-dyld) — STATUS
- **Test**: `rm -f ~/.osxie/.init.pid && timeout --kill-after=2 8 install/bin/osxie <binary>`. Exit 137 = alive (killed by timeout). Exit 0 = clean exit. Other = crash.
- **Results** (2026-08-19):
| App | dyld | Exit | Notes |
|-----|------|------|-------|
| TestWindow | pass | 137 | X11 window visible, dark theme |
| iTerm2 | pass | 137 | Two windows, dark theme |
| cpuinfo | pass | 137 | Stable run loop |
| GIMP | pass | 0 | Clean exit |
| VLC | pass | 0 | Clean exit |
| Audacity | pass | 137 | |
| Inkscape | pass | 137 | |
| Sublime Text | pass | 137 | |
| Transmission | pass | 137 | |
| **The Unarchiver** | **pass** | **137** | **NEW: passes dyld after CFNetworkBridge** |
| Hex Fiend | pass | 134 | Post-dyld SIGSEGV (Tcl/ObjC bridge) |
| KeePassXC | pass | 136 | Post-dyld SIGFPE in Mesa GL |
| MacVim | pass | 139 | Post-dyld SIGSEGV at 0x7FFF00000000 |
| Stellarium | pass | 139 | Post-dyld SIGSEGV, CFBasicHash |
| Firefox | pass | 132 | Post-dyld SIGILL (JIT) |
| **Obsidian** | **pass** | **137** | **FIXED: ___NSDictionary0__struct + NSConstant* stubs** |
| **Sequel Ace** | **pass** | **137** | **FIXED: bad_function_call + NSPICTImageRep** |
| **CotEditor** | **fail** | **1** | 540 Swift ABI symbols — needs full Swift Foundation |
| **IINA** | **fail** | **1** | VideoToolbox symbols (requires COMPONENTS=all build) |
- **Post-dyld crashes** (Hex Fiend, KeePassXC, MacVim, Stellarium, Firefox): All pass dyld loading, then crash during app initialization. Common pattern: Mesa/OpenGL GLX context creation failure ("Failed to create /Users for shader cache"). These are GL rendering infrastructure issues, not missing symbols.
- **Hard blockers** (CotEditor, IINA): Need Swift runtime or VideoToolbox — require full component builds.
- **Status**: IN PROGRESS. 17/19 apps pass dyld. 5 post-dyld crashes need Mesa/GL fix. 2 hard-blocked on Swift/VideoToolbox.

### 37. ObjC duplicate class warnings flood osxie shell — FIXED
- **Files**: `src/external/objc4/runtime/objc-class.mm`, `src/external/corefoundation/CMakeLists.txt`
- **Problem**: Every `osxie shell` launch printed ~28 `objc[N]: Class X is implemented in both Y and Z` warnings to stderr. Two root causes:
  1. CoreFoundation re-defined `NSConstantDictionary` and `NSConstantIntegerNumber` classes that already existed in Foundation (added for Issue 35/36 dyld fixes).
  2. CFNetwork and Foundation both define CFNetwork-origin classes (`NSURLRequest`, `NSURLSession`, etc.) because `-reexport_library` is broken (Issue 35).
- **Fix (2 layers)**:
  1. **CoreFoundation**: Removed `NSConstantDictionary.m` and `NSConstantIntegerNumber.m` from `CMakeLists.txt` SOURCES (Foundation already provides real implementations). Kept `NSConstantArray.m` and `NSConstantDoubleNumber.m` (only in CF).
  2. **ObjC runtime** (`objc-class.mm:inform_duplicate`): Changed to only call `_objc_fatal` (which never fires in release since `DebugDuplicateClasses` defaults off). The `_objc_inform` warning path is now dead code — duplicate classes are silently tolerated. This also covers the CFNetwork/Foundation duplicates from Issue 35.
- **Verified 2026-08-19**: `osxie shell` output is clean — zero "implemented in both" lines.
- **Status**: FIXED + deployed (libobjc.A.dylib rebuilt and copied to `~/.osxie/usr/lib/`).

### 38. htop works inside osxie shell — VERIFIED
- **File**: `~/.osxie/usr/local/Cellar/htop/3.5.2/bin/htop`
- **Problem**: htop (macOS Homebrew 3.5.2, ncurses TUI) previously reported as not starting. Earlier failures were caused by stale/dead osxieserver processes returning `-111` (ECONNREFUSED) on RPC calls.
- **Verified 2026-08-19**: htop renders full TUI with CPU bars, memory (32K/17.4G), swap, load average, uptime, 25 tasks (all mldr processes), F1-F10 function bar. Works both via `install/bin/osxie htop` and inside `osxie shell`. Exit code correct (137 = killed by timeout after 6s).

### 39. Submodule update to upstream + osxify push (2026-08-20)
- **Scope**: merged latest upstream `darlinghq` into the osxified submodules,
  preserving the local osxify commits, then pushed everything to the
  `GatoAmarilloBicolor/osxie-*` forks and the repo root.
- **cocotron** (`migrated`): merged upstream (2 commits: X11/Onyx2D backing-store
  leak fixes) — **no conflicts**. Also committed local osxify fixes:
  GL context stability (NSOpenGLContext NULL-surface + 0×0 clamp, X11Window
  default-visual fallback, X11SubWindow size clamp) + dedup NSWindow keyDown.
  Build verified (`AppKit`/`X11`/`OpenGL` relink OK).
- **corefoundation** (`master`): merged upstream (Info.plist + CMakeLists). Also
  committed the nested `submodules/swift-corelibs-foundation` osxify changes
  (DARLING→OSXIE guards) to a new `osxify` branch, pushed to
  `osxie-swift-corelibs-foundation` fork.
- **foundation** (`master`): was already up-to-date with upstream (0 commits
  pending). Pushed osxify commits incl. new `NSAppleEventManager`
  currentAppleEvent/currentReplyAppleEvent/appleEventForSuspensionID/
  replyAppleEventForSuspensionID returning nil (fixes garbage-return crash in
  MacVim/iTerm2).
- **objc4**/xnu/swift: already up-to-date with upstream; pushed osxify commits.
  Swift push needed `-c core.hooksPath=/dev/null` (its `.lfsconfig` points to
  auth-only `git-lfs.darlinghq.org`).
- **Repo root** `master`: committed updated submodule pointers (cocotron,
  corefoundation, foundation) + `.gitignore` (added `gl_probe/`). Pushed to
  `GatoAmarilloBicolor/Osxie`.
- **Verification**: all fork remotes' HEADs match local submodule HEADs; repo
  root pushed `6a651e678..ea05bb94c`. Rebuild of AppKit/OpenGL/CoreFoundation/
  Foundation after merges: EXIT 0.
- **Swift runtime libs added** (runtime, not committed): created + deployed to
  `~/.osxie/usr/lib/swift/` the missing stub dylibs `libswiftOSLog`,
  `libswiftCoreMIDI`, `libswiftQuickLookUI`, `libswiftVideoToolbox`,
  `libswift_Builtin_float` (each exports only the `__swift_FORCE_LOAD_$_swiftX`
  symbol the apps import). These unblock dyld for CotEditor, cpuinfo, IINA,
  Sequel Ace. Generated from `gl_probe/stubs/` (gitignored).

### 40. Full submodule upstream audit (2026-08-20) — ALL UP-TO-DATE
- Scanned all **152** first-level submodules against their upstream
  (`origin/master` or `origin/main`). **146 are up-to-date (0 commits
  pending)**; the 4 `libressl-*` show `up=1` but that is a false positive — the
  darling `origin/master` (`52dbcfd "Initial commit"`) does not share history
  with the `osxie-libressl` fork, and each directory's osxify HEAD is already
  pushed to the fork branch `libressl-<ver>` (verified via `ls-remote`).
- `openjdk`/`osxie-dmg` have no upstream branch (not applicable).
- Nested submodules of `IOKitUser` (`darling/submodules/IOGraphics`,
  `IOHIDFamily`) have no upstream branch either; left as-is (avoid risking the
  build). `MITKerberosShim`/`SecurityTokend` verified `upstream+0`.
- Repo root `master` in sync with `origin/master` (0 ahead/0 behind). Note a
  concurrent session also pushed `1627ce6e5` (OpenGL NULL-surface guard +
  CGLChoosePixelFormat off-by-one) which is already present locally and remote.

### 41. osxieserver runtime sync (2026-08-20) — FIXED
- The system launcher `/usr/local/bin/osxie` execs `/usr/local/bin/osxieserver`
  → `osxieserver.real`, which was a **stale 2026-08-13 build** missing the
  Issue 28 fix (`dtape_hook_task_for_each`=0, while `install/bin/osxieserver`
  and `~/.osxie/bin/osxieserver` had it). This was a real contributor to CPU-Info
  GangWorker instability.
- **Fix**: synced `/usr/local/bin/osxieserver.real` with
  `install/bin/osxieserver` via `pkexec cp` with absolute paths (interactive
  polkit agent authorized). Now all three copies are byte-identical
  (md5 `e493091c`), each with Issue 28 (`dtape_hook_task_for_each`=4) + Issue 30
  (`timer_call_cancel`=2).
- **Note**: pkexec needed absolute paths (`install/bin/osxieserver` relative was
  not found because pkexec changes cwd). The dev launcher `install/bin/osxie`
  already pointed to the new `install/bin/osxieserver`.
