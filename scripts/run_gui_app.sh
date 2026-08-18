#!/bin/bash
# run_gui_app.sh — painless launcher for GUI apps under osxie.
#
# Solves the dev-loop friction of running X11 GUI apps on this machine:
#   - no `script` wrapper (it hung and left `osxie shell` orphans behind),
#   - timestamped log with the app's full output (stdout+stderr),
#   - hard timeout so a hung app can't stall you,
#   - automatic cleanup of the orphaned guest shells this launch creates
#     (each `osxie <app>` leaks an `osxie shell`; we only kill NEW ones),
#   - clear rc reporting (124 == killed by timeout).
#
# Usage:
#   scripts/run_gui_app.sh <app-binary>            # run once
#   scripts/run_gui_app.sh --timeout 120 <app>     # custom timeout (default 90)
#   scripts/run_gui_app.sh --restart-server <app>  # restart osxieserver first
#   scripts/run_gui_app.sh --no-cleanup <app>      # leave guest shells alone
#   scripts/run_gui_app.sh --log-dir /tmp/x <app>  # where to write the log
#
# Env:
#   DISPLAY      X display to use (default :0)
#   INSTALL_DIR  osxie install tree (default /home/fenix/src/Osxie/install)
#   LOG_DIR      log output dir (default /tmp/opencode)
#   TIMEOUT      default timeout seconds (default 90)

set -u

INSTALL_DIR="${INSTALL_DIR:-/home/fenix/src/Osxie/install}"
LOG_DIR="${LOG_DIR:-/tmp/opencode}"
DISPLAY="${DISPLAY:-:0}"
DEF_TIMEOUT="${TIMEOUT:-90}"
OSXIE_PREFIX="${OSXIE_PREFIX:-$HOME/.osxie}"

TIMEOUT="$DEF_TIMEOUT"
RESTART=0
CLEANUP=1
APP=""

while [ $# -gt 0 ]; do
  case "$1" in
    --timeout) TIMEOUT="${2:?--timeout needs a value}"; shift 2 ;;
    --timeout=*) TIMEOUT="${1#--timeout=}"; shift ;;
    --restart-server|-r) RESTART=1; shift ;;
    --no-cleanup) CLEANUP=0; shift ;;
    --log-dir) LOG_DIR="${2:?--log-dir needs a value}"; shift 2 ;;
    --log-dir=*) LOG_DIR="${1#--log-dir=}"; shift ;;
    --help|-h)
      grep '^# ' "$0" | sed 's/^# \{0,1\}//'
      exit 0 ;;
    --*) echo "unknown arg: $1" >&2; exit 64 ;;
    *) [ -n "$APP" ] && { echo "only one app path allowed" >&2; exit 64; }; APP="$1"; shift ;;
  esac
done

[ -n "$APP" ] || { echo "usage: $0 <app-binary> [--timeout N] [--restart-server] [--no-cleanup]" >&2; exit 64; }
# The app path is a CONTAINER path (e.g. /Applications/foo.app/...), which on
# the host lives under $OSXIE_PREFIX. Accept either spelling for the check.
if [ ! -x "$APP" ] && [ ! -x "$OSXIE_PREFIX$APP" ]; then
  echo "app not executable: $APP (host) / $OSXIE_PREFIX$APP (container)" >&2
  exit 66
fi

if [ ! -t 0 ]; then
  echo "WARN: not running from an interactive session — osxie's setuid launcher may need one" >&2
fi

if [ "$RESTART" -eq 1 ]; then
  pids=$(pgrep -f "osxieserver $OSXIE_PREFIX" 2>/dev/null) || pids=""
  if [ -n "$pids" ]; then
    echo "== restarting osxieserver (pids: $pids)"
    kill $pids 2>/dev/null; sleep 1
  fi
fi

name="$(basename "$APP")"
log="$LOG_DIR/${name}_$(date +%Y%m%d_%H%M%S).log"
mkdir -p "$LOG_DIR"

# Remember guest shells that already exist so cleanup only touches OUR leftovers.
before=$(pgrep -f '^osxie shell$' 2>/dev/null | tr '\n' ' ')
# Same for the app binary itself: the guest process survives the launcher, so
# kill OUR instance, not a pre-existing one.
before_app=$(pgrep -f "$APP" 2>/dev/null | tr '\n' ' ')

cd "$INSTALL_DIR/bin" || { echo "no such dir: $INSTALL_DIR/bin" >&2; exit 2; }

echo "== launching: DISPLAY=$DISPLAY ./osxie $APP"
echo "== log -> $log"
# -k 5: the osxie launcher ignores SIGTERM and would stall `timeout` forever;
# SIGKILL the process group 5s later so the script can't hang.
timeout -k 5 "$TIMEOUT" ./osxie "$APP" > "$log" 2>&1
rc=$?

if [ "$rc" -eq 124 ] || [ "$rc" -eq 137 ]; then
  echo "== TIMEOUT after ${TIMEOUT}s — app killed by timeout (rc=$rc)"
elif [ "$rc" -eq 0 ]; then
  echo "== app exited rc=0"
else
  echo "== app exited rc=$rc"
fi

# The guest process outlives the launcher; sweep OUR leftover instance.
after_app=$(pgrep -f "$APP" 2>/dev/null | tr '\n' ' ')
for p in $after_app; do
  case " $before_app " in *" $p "*) ;; *) echo "== killing leftover guest $p ($APP)"; kill -9 "$p" 2>/dev/null ;; esac
done

if [ "$CLEANUP" -eq 1 ]; then
  after=$(pgrep -f '^osxie shell$' 2>/dev/null | tr '\n' ' ')
  for p in $after; do
    case " $before " in *" $p "*) ;; *) echo "== cleaning up orphaned guest shell $p"; kill "$p" 2>/dev/null ;; esac
  done
fi

echo "== recent output =="
tr -d '\r' < "$log" | grep -vE '^$' | tail -30
exit 0
