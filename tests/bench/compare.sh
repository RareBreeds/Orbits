#!/usr/bin/env bash
# Compares the benchmarks of the working tree against another revision of the plugin.
#
#   RACK_DIR=<path> tests/bench/compare.sh [revision]
#
# The revision defaults to the last release tag (v*, not a -dev version). Only the plugin's
# sources come from that revision: the benchmarks themselves are the working tree's, so both
# sides run the same scenarios, and revisions from before the benchmarks existed still work. The
# working tree includes uncommitted changes.
#
# Timings vary from machine to machine and with what else the machine is doing, so both builds
# run here, alternating, ROUNDS times each (default 3). The best time of all the rounds is
# compared for each scenario (the best median, for drawing). Each build draws with its own
# revision's SVGs and fonts. A Rack window opens while the benchmarks run.
set -euo pipefail

if [ -z "${RACK_DIR:-}" ]; then
        echo "Set RACK_DIR to a Rack SDK or Rack source directory, e.g. RACK_DIR=\$HOME/Rack-SDK $0" >&2
        exit 1
fi
rounds=${ROUNDS:-3}

tests_dir=$(cd "$(dirname "$0")/.." && pwd)
repo_dir=$(git -C "$tests_dir" rev-parse --show-toplevel)
base=${1:-$(git -C "$repo_dir" describe --tags --abbrev=0 --match 'v*' --exclude '*-dev*')}
base_commit=$(git -C "$repo_dir" rev-parse --verify --quiet "$base^{commit}") || {
        echo "Unknown revision: $base" >&2
        exit 1
}

work_dir=$(mktemp -d)
cleanup()
{
        git -C "$repo_dir" worktree remove --force "$work_dir/src-tree" 2>/dev/null || true
        rm -rf "$work_dir"
}
trap cleanup EXIT

echo "Building the working tree"
make -C "$tests_dir" --no-print-directory -s RACK_DIR="$RACK_DIR" build/orbits-bench
current_bench="$tests_dir/build/orbits-bench"

echo "Building $base ($(git -C "$repo_dir" rev-parse --short "$base_commit"))"
git -C "$repo_dir" worktree add --detach --quiet "$work_dir/src-tree" "$base_commit"
# Older sources weren't written for today's warnings, so don't show them.
make -C "$tests_dir" --no-print-directory -s RACK_DIR="$RACK_DIR" BUILD_DIR="$work_dir/build" \
        BENCH_SRC_DIR="$work_dir/src-tree/src" WARNINGS= "$work_dir/build/orbits-bench"
base_bench="$work_dir/build/orbits-bench"

# libRack's install name on macOS is just "libRack.dylib", so tell the loader where it is.
export DYLD_LIBRARY_PATH="$RACK_DIR" LD_LIBRARY_PATH="$RACK_DIR"
for round in $(seq "$rounds"); do
        echo "Round $round of $rounds"
        "$base_bench" "$RACK_DIR" "$work_dir/src-tree" >> "$work_dir/base.txt"
        "$current_bench" "$RACK_DIR" "$repo_dir" >> "$work_dir/current.txt"
done

# Each table starts with a heading line, then has a line per scenario: a 50 character name, then
# numbers, of which the second to last is the best time. The output keeps the tables' order.
awk -v base="$base" '
        FNR == 1 { file++ }
        /^$/ { next }
        $NF !~ /^-?[0-9.]+$/ {
                heading = substr($0, 1, 50)
                sub(/ +$/, "", heading)
                if(file == 1 && !(heading in headings))
                {
                        headings[heading] = 1
                        order[++count] = "\n" heading
                }
                next
        }
        {
                name = substr($0, 1, 50)
                sub(/ +$/, "", name)
                key = heading SUBSEP name
                best = $(NF - 1) + 0
                if(file == 1)
                {
                        if(!(key in base_best))
                        {
                                order[++count] = key
                        }
                        if(!(key in base_best) || best < base_best[key])
                        {
                                base_best[key] = best
                        }
                }
                else if(!(key in current_best) || best < current_best[key])
                {
                        current_best[key] = best
                }
        }
        END {
                for(i = 1; i <= count; i++)
                {
                        key = order[i]
                        if(substr(key, 1, 1) == "\n")
                        {
                                printf "\n%-50s %8s %8s %8s\n", substr(key, 2), base, "current", "change"
                                continue
                        }
                        split(key, parts, SUBSEP)
                        if(!(key in current_best))
                        {
                                printf "%-50s %8.1f %8s\n", parts[2], base_best[key], "-"
                                continue
                        }
                        change = base_best[key] != 0 ? sprintf("%+7.0f%%", 100 * (current_best[key] - base_best[key]) / base_best[key]) : ""
                        printf "%-50s %8.1f %8.1f %8s\n", parts[2], base_best[key], current_best[key], change
                }
        }
' "$work_dir/base.txt" "$work_dir/current.txt"
