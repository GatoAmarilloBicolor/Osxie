#!/bin/bash
# run_cpuinfo.sh — run cpuinfo once, capturing the NIB-connection trace that
# pinpoints the crashing connector. Uses the instrumented AppKit/Foundation that
# scripts/relink_dylibs.sh already installed into ~/.osxie.
#
# MUST be run from an interactive session (osxie/`osxie` launcher is setuid and
# needs a real terminal + the GUI session).
set -u
OUT="${1:-/tmp/opencode/cpuinfo_latest.log}"
cd /home/fenix/src/Osxie/install/bin || exit 2
echo "== running cpuinfo, full log -> $OUT"
timeout 90 ./osxie /Applications/cpuinfo.app/Contents/MacOS/cpuinfo > "$OUT" 2>&1
echo "rc=$?"
echo "=== recent trace ==="
grep -E "establishConnections|buildConnections|decode IB|delivering|Segmentation|finishLaunching" "$OUT" | tail -30
echo "=== crashing connector line (grep before 'fatal'/'mtraces') ==="
grep -E "class=NSNib|last exception|Traceback|crashed" "$OUT" | tail -10