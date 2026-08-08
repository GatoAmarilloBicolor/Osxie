#!/bin/bash
# relink_dylibs.sh — fast dev-loop that rebuilds JUST the OSXIE frameworks you
# are iterating on and drops them straight into the runtime prefix (~/.osxie).
#
# Purpose: the full build_complete.sh runs `pkexec ninja install`, which is slow
# and (because it rewrites the whole /usr/local/libexec tree) would re-touch the
# setuid-root shellspawn. That is risky and unnecessary when you only changed a
# high-level framework like AppKit/Foundation. This script:
#   - builds only the requested dylib targets,
#   - copies them directly into ~/.osxie/System/Library/Frameworks/...,
#   - ALSO copies AppKit's X11 backend (the window/tray machinery lives there),
#   - NEVER runs setuid, sudo, or pkexec, and NEVER reinstalls shellspawn.
#
# Runtime prefix — ONE copy matters. Verified via /proc/<app>/maps: the running
# app loads frameworks ONLY from $OSXIE_PREFIX/System/Library/Frameworks/...
# The tree at $OSXIE_PREFIX/libexec/osxie/System/... is a stale full install
# that the app NEVER consults, so this script does NOT touch it.
#
# Usage:
#   scripts/relink_dylibs.sh                 # rebuild+copy AppKit & Foundation (default)
#   scripts/relink_dylibs.sh --restart-server# also restart osxieserver
#   scripts/relink_dylibs.sh AppKit Foundation CoreFoundation
#   scripts/relink_dylibs.sh AppKit          # just one framework
#
# Env:
#   BUILD_DIR   build tree (default build_new)
#   OSXIE_PREFIX  runtime prefix (default ~/.osxie)

set -u
BUILD_DIR="${BUILD_DIR:-build_new}"
OSXIE_PREFIX="${OSXIE_PREFIX:-$HOME/.osxie}"

# map: framework name -> rel-path of the built dylib under the cmake build dir.
declare -A FRAME_DYLIB=(
  [AppKit]=src/external/cocotron/AppKit/AppKit
  [Foundation]=src/external/foundation/Foundation
  [CoreFoundation]=src/external/corefoundation/CoreFoundation
  [CoreServices]=src/external/coreservices/CoreServices
  [Security]=src/external/security/Security
)

# ninja target + destination for AppKit's X11 backend (window/tray backend).
X11_BACKEND_TARGET=X11_backend
X11_BACKEND_SRC="$BUILD_DIR/src/external/cocotron/AppKit/X11"
X11_BACKEND_DEST="$OSXIE_PREFIX/System/Library/Frameworks/AppKit.framework/Versions/C/Resources/Backends/X11.backend/Contents/MacOS/X11"

RESTART=0
FRAMES=()
for a in "$@"; do
  case "$a" in
    --restart-server|-r) RESTART=1 ;;
    --*) echo "unknown arg: $a" >&2; exit 64 ;;
    *) FRAMES+=("$a") ;;
  esac
done
[ "${#FRAMES[@]}" -eq 0 ] && FRAMES=(AppKit Foundation)

CCTOOLS_BIN="$BUILD_DIR/src/external/cctools-port/cctools"
export PATH="$CCTOOLS_BIN/misc:$CCTOOLS_BIN/ar:$CCTOOLS_BIN/ld64/src:$CCTOOLS_BIN/libstuff:$PATH"

# Wait out any concurrent ninja in the same tree (two ninjas corrupt each other).
while pgrep -x ninja >/dev/null 2>&1; do
  echo "waiting for another ninja to finish..." ; sleep 15
done

failed=0
for f in "${FRAMES[@]}"; do
  [ -n "${FRAME_DYLIB[$f]:-}" ] || { echo "no dylib mapping for '$f'" >&2; failed=1; continue; }
  rel="${FRAME_DYLIB[$f]}"
  echo "== building $f ($BUILD_DIR/$rel) =="
  if ! ninja -C "$BUILD_DIR" "$rel"; then
    echo "BUILD FAILED for $f" >&2; failed=1; continue
  fi
  dest="$OSXIE_PREFIX/System/Library/Frameworks/$f.framework/Versions/C/$f"
  echo "== copying to $dest"
  cp "$BUILD_DIR/$rel" "$dest" || failed=1

  # AppKit's window/tray backend lives under Resources/Backends/ — rebuild and
  # copy it together with AppKit, otherwise the new AppKit still runs the stale
  # X11 backend from the previous iteration.
  if [ "$f" = "AppKit" ]; then
    echo "== building $X11_BACKEND_TARGET (AppKit backend)"
    if ! ninja -C "$BUILD_DIR" "$X11_BACKEND_TARGET"; then
      echo "BUILD FAILED for $X11_BACKEND_TARGET" >&2; failed=1; continue
    fi
    echo "== copying backend to $X11_BACKEND_DEST"
    cp "$X11_BACKEND_SRC" "$X11_BACKEND_DEST" || failed=1
  fi
done

if [ "$failed" -ne 0 ]; then
  echo "ERROR: one or more frameworks failed (see above)" >&2
  exit 1
fi

if [ "$RESTART" -eq 1 ]; then
  # Restart osxieserver so the freshly copied dylibs are mapped by the next app
  # launch. osxieserver is user-owned (no setuid), so plain kill is fine.
  pids=$(pgrep -f "osxieserver $OSXIE_PREFIX" 2>/dev/null) || pids=""
  if [ -n "$pids" ]; then
    echo "== restarting osxieserver (pids: $pids)"
    kill $pids 2>/dev/null
    sleep 1
  fi
  # osxieserver will be re-spawned lazily on the next `osxie` launch.
fi

echo "DONE: relinked ${FRAMES[*]} into $OSXIE_PREFIX"
echo "Next:  cd /home/fenix/src/Osxie/install/bin && ./osxie /Applications/cpuinfo.app/Contents/MacOS/cpuinfo"