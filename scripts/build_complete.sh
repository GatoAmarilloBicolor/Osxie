#!/bin/bash
# build_complete.sh — single-command Osxie full build.
#
# Resumes/completes the full build in build-all, survives OOM (machine shared
# with other agents: 17Gi RAM, ~12Gi already in use), then installs via sudo/
# pkexec. Optionally forks+pushes the osxified submodules to the user's GitHub.
#
# Usage:
#   scripts/build_complete.sh                 # build + install
#   scripts/build_complete.sh --no-install    # build only
#   scripts/build_complete.sh --osxify-push   # build + install + fork/push submodules
#   scripts/build_complete.sh --jobs N        # force parallelism (default: auto 1/2)
#   scripts/build_complete.sh --max-attempts N
#
# Logs: /tmp/opencode/build-retry.log  (per-attempt "=== ATTEMPT N ... ===")

set -u
BUILD_DIR="${BUILD_DIR:-build_new}"
LOG="${BUILD_LOG:-/tmp/opencode/build-retry.log}"
MAX_ATTEMPTS="${MAX_ATTEMPTS:-40}"
INSTALL=1
OSXIFY_PUSH=0
JOBS="auto"

# Canonical tree is Ninja (build_new per AGENTS.md); fall back to make for
# Makefile-based trees.
if [ -f "$BUILD_DIR/build.ninja" ]; then
  BUILDER="ninja"
elif [ -f "$BUILD_DIR/Makefile" ]; then
  BUILDER="make"
else
  echo "== ERROR: no build.ninja nor Makefile in $BUILD_DIR ==" | tee -a "$LOG"
  exit 2
fi

for a in "$@"; do
  case "$a" in
    --no-install) INSTALL=0 ;;
    --osxify-push) OSXIFY_PUSH=1 ;;
    --jobs=*) JOBS="${a#--jobs=}" ;;
    --max-attempts=*) MAX_ATTEMPTS="${a#--max-attempts=}" ;;
    *) echo "unknown arg: $a" >&2; exit 64 ;;
  esac
done

mkdir -p "$(dirname "$LOG")"
echo "== build_complete: $(date) install=$INSTALL osxify=$OSXIFY_PUSH jobs=$JOBS ==" >> "$LOG"

mem_avail_gb() { awk '/MemAvailable/{printf "%.1f", $2/1024/1024}' /proc/meminfo; }
AVAIL=$(mem_avail_gb)
if [ "$JOBS" = "auto" ]; then
  JOBS=2
  if awk -v a="$AVAIL" 'BEGIN{exit !(a<8)}'; then JOBS=1; fi
fi
echo "== available mem: ${AVAIL}Gi -> jobs=$JOBS (ulimit -v 6Gi cap) ==" >> "$LOG"

# Per-process memory cap: guarantees the build itself never triggers a system
# OOM, so the retry shell is never the OOM killer's victim. Observed max TU is
# ~2.5Gi (LowLevelInterpreter.cpp.o); 6Gi is a generous ceiling. A TU that hits
# the cap dies with a make error and is simply retried.
ulimit -v 6291456 2>/dev/null

# clang's Darwin driver spawns lipo/ld/ar/ranlib by name for fat-object compile
# steps and links; they are built under cctools-port and must be on PATH.
CCTOOLS_BIN="$BUILD_DIR/src/external/cctools-port/cctools"
export PATH="$CCTOOLS_BIN/misc:$CCTOOLS_BIN/ar:$CCTOOLS_BIN/ld64/src:$CCTOOLS_BIN/libstuff:$PATH"

attempt=0
ATT=$(mktemp /tmp/opencode/build-attempt.XXXXXX.log)
while :; do
  attempt=$((attempt + 1))
  [ "$attempt" -gt "$MAX_ATTEMPTS" ] && { echo "EXHAUSTED $MAX_ATTEMPTS attempts" >> "$LOG"; exit 2; }
  echo "=== ATTEMPT $attempt $(date) (jobs=$JOBS) ===" >> "$LOG"
  : > "$ATT"
  # Wait out any concurrent ninja from another agent in the same tree — two
  # ninjas in one tree corrupt each other (missing .o.d dirs, posix_spawn
  # ENOENT in clang's lipo sub-steps).
  while pgrep -x ninja >/dev/null 2>&1; do
    echo "  waiting for another ninja to finish..." >> "$ATT"
    sleep 30
  done
  if [ "$BUILDER" = "ninja" ]; then
    ninja -C "$BUILD_DIR" -j"$JOBS" >> "$ATT" 2>&1
  else
    make -C "$BUILD_DIR" -j"$JOBS" >> "$ATT" 2>&1
  fi
  rc=$?
  cat "$ATT" >> "$LOG"
  echo "=== attempt $attempt rc=$rc ===" >> "$LOG"
  [ "$rc" -eq 0 ] && break

  # OOM heuristics on THIS attempt only (greping the cumulative log would
  # misfire on stale "Terminado" lines and hide real link/compile errors).
  if grep -qE "Terminado|Killed|bad_alloc|MemorySanitizer|out of memory|posix_spawn failed|errno=12|can't map file" "$ATT"; then
    # TCP slow-start ladder: 6 -> 4 -> 2 -> 1 after each OOM.
    JOBS=$((JOBS > 4 ? JOBS/2 : (JOBS == 4 ? 2 : (JOBS == 2 ? 1 : 1))))
    echo "OOM-killed; jobs->$JOBS, retrying" >> "$LOG"
    sleep 10
    continue
  fi
  echo "NON-OOM FAILURE (rc=$rc) at attempt $attempt — last error:" >> "$LOG"
  grep -E "error:|fatal error|Error [0-9]|undefined reference|Undefined symbols|clang.*error" "$ATT" | tail -3 >> "$LOG"
  exit 3
done
rm -f "$ATT"

echo "BUILD SUCCEEDED ($attempt attempts, $(date))" >> "$LOG"

install() {
  ABS_DIR="$(cd "$BUILD_DIR" && pwd)"
  if sudo -n true 2>/dev/null; then
    echo "installing via sudo..." | tee -a "$LOG"
    if [ "$BUILDER" = "ninja" ]; then
      sudo ninja -C "$ABS_DIR" install >> "$LOG" 2>&1 || exit 4
    else
      sudo make -C "$ABS_DIR" install >> "$LOG" 2>&1 || exit 4
    fi
  else
    echo "sudo needs a password — installing via pkexec" | tee -a "$LOG"
    if [ "$BUILDER" = "ninja" ]; then
      pkexec ninja -C "$ABS_DIR" install >> "$LOG" 2>&1 || exit 4
    else
      pkexec make -C "$ABS_DIR" install >> "$LOG" 2>&1 || exit 4
    fi
  fi
  echo "INSTALL SUCCEEDED" >> "$LOG"
}
[ "$INSTALL" -eq 1 ] && install

if [ "$OSXIFY_PUSH" -eq 1 ]; then
  echo "== osxify-push step ==" | tee -a "$LOG"
  # Submodules genuinely modified by the osxification (guards DARLING->OSXIE,
  # darling/ dir removal, SDK framework relinks). Everything else in `git submodule
  # status` is build/junk and is intentionally left untouched.
  for sub in src/external/OpenLDAP src/external/python src/external/JavaScriptCore \
             src/external/Heimdal src/external/security; do
    echo "-- $sub --" | tee -a "$LOG"
    if [ -z "$(git -C "$sub" diff --stat)" ]; then echo "   no tracked changes; skip"; continue; fi
    git -C "$sub" add -u || { echo "   add failed"; exit 5; }
    git -C "$sub" commit -q -m "osxify: replace DARLING guards/refs with OSXIE (Osxie fork)" || echo "   nothing to commit"
    git -C "$sub" checkout -B osxie >/dev/null 2>&1
    git -C "$sub" remote remove osxie >/dev/null 2>&1 || true
    git -C "$sub" remote add osxie "https://github.com/GatoAmarilloBicolor/osxie-${sub#src/external/}.git" >/dev/null 2>&1 || true
    git -C "$sub" push -u osxie osxie >> "$LOG" 2>&1 || echo "   push FAILED (see log)"
    echo "   pushed osxie -> osxie-${sub#src/external/}" | tee -a "$LOG"
  done
  # Parent repo: SDK/regenerated framework changes (Developer is not a submodule).
  git add -u Developer 2>/dev/null
  git add cmake CMakeLists.txt scripts 2>/dev/null
  git commit -q -m "osxie: SDK framework relinks (darling->osxie), build automation" 2>/dev/null || echo "   parent: nothing new to commit"
  git push >> "$LOG" 2>&1 || echo "   parent push FAILED (see log)"
fi

echo "== DONE $(date) ==" | tee -a "$LOG"
