#!/usr/bin/env bash
# Packs ChillSpawn into dist/@ChillSpawn/addons/ChillSpawn.pbo on Linux, using
# the PBO builder from RaG DayZ Tools (https://github.com/Tyson89/RaG-DayZ-Tools).
#
# RaG DayZ Tools is freeware under its own licence and is not part of this
# repository. This script uses a separate clone of it: pass its path, set
# RAG_TOOLS_DIR, or let the script clone it into ~/.cache/rag-dayz-tools.
#
# Usage: tools/build-linux.sh [path-to-RaG-DayZ-Tools]
#
# Needs python3 (3.10+) and git. Binarise, config conversion and signing need
# Windows-only DayZ Tools programs, so they are switched off; a scripts-only
# server mod doesn't need them.
set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
rag="${1:-${RAG_TOOLS_DIR:-$HOME/.cache/rag-dayz-tools}}"

if [ ! -f "$rag/rag_pbo_builder_cli.py" ]; then
	if [ $# -ge 1 ] || [ -n "${RAG_TOOLS_DIR:-}" ]; then
		echo "error: $rag does not contain rag_pbo_builder_cli.py" >&2
		exit 1
	fi
	echo "Cloning RaG DayZ Tools into $rag"
	git clone --depth 1 https://github.com/Tyson89/RaG-DayZ-Tools "$rag"
fi

work="$repo/dist/.build"
out="$repo/dist/@ChillSpawn"
rm -rf "$work" "$out"
mkdir -p "$work/source" "$work/temp" "$work/appdata"

# Build from a copy so the builder's caches never land in the source tree.
cp -r "$repo/ChillSpawn" "$work/source/"

# The builder keeps settings and logs under LOCALAPPDATA; keep them in dist/.
LOCALAPPDATA="$work/appdata" python3 -I - "$rag" build \
	--source "$work/source" \
	--project-root "$work/source" \
	--temp "$work/temp" \
	--output "$out" \
	--addons ChillSpawn \
	--no-binarize --no-convert-config --no-update-paa --no-sign \
	--preflight --force <<'EOF'
import sys
sys.path.insert(0, sys.argv.pop(1))
import rag_pbo_builder_cli as cli
sys.exit(cli.run_cli(sys.argv[1:]))
EOF

# Linux servers are case-sensitive; DayZ expects a lowercase addons folder.
mv "$out/Addons" "$out/addons"
rmdir "$out/Keys" 2>/dev/null || true

echo
echo "Built $out/addons/ChillSpawn.pbo"
echo "Copy dist/@ChillSpawn into the server folder and add -servermod=@ChillSpawn to the start line."
