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
- **cpuinfo** (menu-bar app, no own window) runs to a stable run loop: `NSIBObjectData initWithCoder: done objects=27 conns=26`, all 26 `establishConnections` done, `NSApp run: finishLaunching returned`, repeated `NSStatusItem button: cache hit` — **0 SIGEXC**. Objects 168–179 are the 12 NSNumbers decoded *before* the NSMutableSet (idx=5) that used to crash.
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
## Build: CORE COMPLETE, READY FOR INSTALL
