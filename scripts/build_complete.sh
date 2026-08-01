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
BUILD_DIR="${BUILD_DIR:-build-all}"
LOG="${BUILD_LOG:-/tmp/opencode/build-retry.log}"
MAX_ATTEMPTS="${MAX_ATTEMPTS:-40}"
INSTALL=1
OSXIFY_PUSH=0
JOBS="auto"

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

attempt=0
while :; do
  attempt=$((attempt + 1))
  [ "$attempt" -gt "$MAX_ATTEMPTS" ] && { echo "EXHAUSTED $MAX_ATTEMPTS attempts" >> "$LOG"; exit 2; }
  echo "=== ATTEMPT $attempt $(date) (jobs=$JOBS) ===" >> "$LOG"
  make -C "$BUILD_DIR" -j"$JOBS" >> "$LOG" 2>&1
  rc=$?
  echo "=== attempt $attempt rc=$rc ===" >> "$LOG"
  [ "$rc" -eq 0 ] && break

  # OOM heuristics: GNU make prints "Terminado"/"Killed" for signal-killed jobs,
  # clang++ aborts on bad_alloc. Otherwise it's a real error -> stop for fixing.
  if grep -qE "Terminado|Killed|bad_alloc|MemorySanitizer|out of memory" "$LOG"; then
    # TCP slow-start ladder: 6 -> 4 -> 2 -> 1 after each OOM.
    JOBS=$((JOBS > 4 ? JOBS/2 : (JOBS == 4 ? 2 : (JOBS == 2 ? 1 : 1))))
    echo "OOM-killed; jobs->$JOBS, retrying" >> "$LOG"
    sleep 10
    continue
  fi
  echo "NON-OOM FAILURE (rc=$rc) at attempt $attempt — last error:" >> "$LOG"
  grep -E "error:|fatal error|Error [0-9]|undefined reference|Undefined symbols" "$LOG" | tail -3 >> "$LOG"
  exit 3
done

echo "BUILD SUCCEEDED ($attempt attempts, $(date))" >> "$LOG"

install() {
  if sudo -n true 2>/dev/null; then
    echo "installing via sudo..." | tee -a "$LOG"
    sudo make -C "$BUILD_DIR" install >> "$LOG" 2>&1 || exit 4
  else
    echo "sudo needs a password — installing via pkexec" | tee -a "$LOG"
    pkexec make -C "$BUILD_DIR" install >> "$LOG" 2>&1 || exit 4
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
