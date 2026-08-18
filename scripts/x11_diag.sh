#!/bin/bash
# x11_diag.sh — one-shot health check for the X11 GUI layer (host + runtime prefix).
#
# Verifies, in order:
#   1. DISPLAY env + the host X socket for it
#   2. the X server is actually reachable (xdpyinfo, if installed)
#   3. the CoreGraphics backend bundle exists in the runtime prefix
#      (both root Resources/Backends and Versions/A + Versions/C)
#   4. the AppKit X11 backend bundle exists
#   5. framework layout sanity (CoreGraphics Versions/A vs Versions/C — the app
#      loads Versions/A, but relink copies to Versions/C; a mismatched mtime is
#      the #1 source of "I rebuilt it but nothing changed")
#   6. osxieserver is running for the prefix
#
# Usage:
#   scripts/x11_diag.sh [--display :0]
#
# Env:
#   DISPLAY       display to probe (default :0)
#   OSXIE_PREFIX  runtime prefix (default ~/.osxie)

set -u

OSXIE_PREFIX="${OSXIE_PREFIX:-$HOME/.osxie}"
DISPLAY="${DISPLAY:-:0}"

ok=0; warn=0; bad=0
say()  { printf '%-10s %s\n' "$1" "$2"; }
pass() { say "OK:" "$1"; ok=$((ok+1)); }
warnf(){ say "WARN:" "$1"; warn=$((warn+1)); }
fail() { say "FAIL:" "$1"; bad=$((bad+1)); }

echo "== X11 GUI layer diagnostics (prefix=$OSXIE_PREFIX, display=$DISPLAY) =="

# 1. DISPLAY + socket
if [ -n "${DISPLAY:-}" ]; then
  n="${DISPLAY##*:}"
  n="${n%%.*}"
  if [ -e "/tmp/.X11-unix/X${n}" ]; then
    pass "DISPLAY=$DISPLAY, host socket /tmp/.X11-unix/X${n} present"
  else
    fail "DISPLAY=$DISPLAY but no socket /tmp/.X11-unix/X${n} on host"
  fi
else
  fail "DISPLAY is not set"
fi

# 2. X server reachable
if command -v xdpyinfo >/dev/null 2>&1; then
  if xdpyinfo -display "$DISPLAY" >/dev/null 2>&1; then
    res=$(xdpyinfo -display "$DISPLAY" | awk '/dimensions/{print $2; exit}')
    pass "X server reachable on $DISPLAY (${res:-unknown})"
  else
    fail "X server on $DISPLAY not reachable (xdpyinfo failed)"
  fi
else
  warnf "xdpyinfo not installed — skipping X reachability probe"
fi

# 3. CoreGraphics backend bundle
cg="$OSXIE_PREFIX/System/Library/Frameworks/CoreGraphics.framework"
cg_be_root="$cg/Resources/Backends/X11.backend/Contents/MacOS/X11"
cg_be_a="$cg/Versions/A/Resources/Backends/X11.backend/Contents/MacOS/X11"
cg_be_c="$cg/Versions/C/Resources/Backends/X11.backend/Contents/MacOS/X11"
if [ -x "$cg_be_root" ]; then
  pass "CoreGraphics backend: root Resources/Backends ($(stat -c %y "$cg_be_root" | cut -c1-16))"
else
  fail "CoreGraphics backend missing: $cg_be_root"
fi
for v in A C; do
  eval "be=\$cg_be_$(echo $v | tr A-Z a-z)"
  [ -x "$be" ] || fail "CoreGraphics backend missing: $be"
done
[ -f "$cg/Versions/A/CoreGraphics" ] || fail "CoreGraphics binary missing in Versions/A"
[ -f "$cg/Versions/C/CoreGraphics" ] || fail "CoreGraphics binary missing in Versions/C"

# 4. AppKit backend bundle
ak="$OSXIE_PREFIX/System/Library/Frameworks/AppKit.framework"
ak_be="$ak/Versions/C/Resources/Backends/X11.backend/Contents/MacOS/X11"
if [ -x "$ak_be" ]; then
  pass "AppKit backend: $ak_be ($(stat -c %y "$ak_be" | cut -c1-16))"
else
  fail "AppKit backend missing: $ak_be"
fi

# 5. Framework binary staleness A vs C (the app loads Versions/A)
if [ -f "$cg/Versions/A/CoreGraphics" ] && [ -f "$cg/Versions/C/CoreGraphics" ]; then
  a=$(stat -c %Y "$cg/Versions/A/CoreGraphics")
  c=$(stat -c %Y "$cg/Versions/C/CoreGraphics")
  if [ "$a" -ne "$c" ]; then
    older=$([ "$a" -lt "$c" ] && echo "Versions/A (the loaded one)" || echo "Versions/C")
    warnf "CoreGraphics A/C mtimes differ — $older is older. The app loads Versions/A."
  else
    pass "CoreGraphics A and C binaries in sync"
  fi
fi

# 6. osxieserver
pids=$(pgrep -f "osxieserver $OSXIE_PREFIX" 2>/dev/null) || pids=""
if [ -n "$pids" ]; then
  pass "osxieserver running (pids: ${pids//$'\n'/ })"
else
  warnf "osxieserver not running — it will be spawned on next osxie launch"
fi

echo
echo "== summary: $ok ok, $warn warnings, $bad failures =="
[ "$bad" -eq 0 ] || exit 1
