#!/usr/bin/env bash
# Compares the modules' behaviour in the working tree with another revision of the plugin.
#
#   RACK_DIR=<path> tests/compare/compare.sh [revision] [scenario name filter]
#
# The revision defaults to the last release tag (v*, not a -dev version). Like the benchmarks'
# compare.sh, only the plugin's sources come from that revision: the scenarios are the working
# tree's, built against each revision's sources, so both run the same patches. The working tree
# includes uncommitted changes.
#
# Each scenario writes a log of everything its module's outputs did, sample by sample, and the logs
# must be the same. Scenarios that test a change made on purpose since the last release say so, and
# their differences are expected.
#
# Every scenario saves and reopens its patch at least once, at the end. The other revision's build
# also writes those patches to files, and the working tree runs the scenarios a second time loading
# them instead of its own, so patches saved with that revision must load and carry on the same.
#
# Last, the working tree runs the scenarios again built with AddressSanitizer and
# UndefinedBehaviorSanitizer, and loads patches with randomly invalid settings, which older revisions
# can crash on. Any output other than 0V or 10V also fails a scenario.
#
# The logs and patches are kept in tests/build/compare/ for looking at with a diff tool. Nothing
# opens a window, so this also runs without a display.
set -euo pipefail

if [ -z "${RACK_DIR:-}" ]; then
        echo "Set RACK_DIR to a Rack SDK or Rack source directory, e.g. RACK_DIR=\$HOME/Rack-SDK $0" >&2
        exit 1
fi

tests_dir=$(cd "$(dirname "$0")/.." && pwd)
repo_dir=$(git -C "$tests_dir" rev-parse --show-toplevel)
release=$(git -C "$repo_dir" describe --tags --abbrev=0 --match 'v*' --exclude '*-dev*')
base=${1:-$release}
filter=${2:-}
base_commit=$(git -C "$repo_dir" rev-parse --verify --quiet "$base^{commit}") || {
        echo "Unknown revision: $base" >&2
        exit 1
}

output_dir="$tests_dir/build/compare"
work_dir=$(mktemp -d)
cleanup()
{
        git -C "$repo_dir" worktree remove --force "$work_dir/src-tree" 2>/dev/null || true
        rm -rf "$work_dir"
}
trap cleanup EXIT

echo "Building the working tree"
make -C "$tests_dir" --no-print-directory -s RACK_DIR="$RACK_DIR" build/orbits-compare
current_compare="$tests_dir/build/orbits-compare"

echo "Building $base ($(git -C "$repo_dir" rev-parse --short "$base_commit"))"
git -C "$repo_dir" worktree add --detach --quiet "$work_dir/src-tree" "$base_commit"
# Older sources weren't written for today's warnings, so don't show them.
make -C "$tests_dir" --no-print-directory -s RACK_DIR="$RACK_DIR" BUILD_DIR="$work_dir/build" \
        COMPARE_SRC_DIR="$work_dir/src-tree/src" WARNINGS= "$work_dir/build/orbits-compare"
base_compare="$work_dir/build/orbits-compare"

rm -rf "$output_dir"
mkdir -p "$output_dir/base" "$output_dir/base-patches" "$output_dir/current" "$output_dir/loaded"
# libRack's install name on macOS is just "libRack.dylib", so tell the loader where it is.
export DYLD_LIBRARY_PATH="$RACK_DIR" LD_LIBRARY_PATH="$RACK_DIR"
echo "Running the scenarios"
# Both builds load the working tree's factory presets.
presets=(--presets "$repo_dir/presets")
"$base_compare" "$output_dir/base" "$filter" "${presets[@]}" --save-patches "$output_dir/base-patches"
"$current_compare" "$output_dir/current" "$filter" "${presets[@]}"
"$current_compare" "$output_dir/loaded" "$filter" "${presets[@]}" --load-patches "$output_dir/base-patches"

# Compares the logs in $output_dir/base with those in $output_dir/$1, counting unexpected
# differences in `failures`. Expected differences are differences from the last release. Comparing
# with another revision, they're only shown.
failures=0
compare_logs()
{
        local current name base_log expectation expected_difference
        for current in "$output_dir/$1"/*.txt; do
                name=$(basename "$current" .txt)
                base_log="$output_dir/base/$name.txt"
                expectation=$(head -n 1 "$current")
                expected_difference=
                case "$expectation" in
                "# Expected to differ"*) expected_difference=${expectation#*: } ;;
                esac

                if cmp -s "$base_log" "$current"; then
                        if [ -n "$expected_difference" ] && [ "$base" = "$release" ]; then
                                printf '%-40s same, but expected to differ: %s\n' "$name" "$expected_difference"
                        else
                                printf '%-40s same\n' "$name"
                        fi
                elif [ -n "$expected_difference" ]; then
                        printf '%-40s different, as expected: %s\n' "$name" "$expected_difference"
                else
                        printf '%-40s DIFFERENT\n' "$name"
                        # diff fails when the files differ, which mustn't end the script.
                        diff "$base_log" "$current" | head -n 20 | sed 's/^/        /' || true
                        failures=$((failures + 1))
                fi
        done
}

echo
echo "The working tree compared with $base:"
compare_logs current
echo
echo "The working tree loading patches saved with $base, compared with $base:"
compare_logs loaded

echo
echo "Checking the working tree with AddressSanitizer and UndefinedBehaviorSanitizer"
sanitized_dir="$tests_dir/build/compare-sanitized"
make -C "$tests_dir" --no-print-directory -s RACK_DIR="$RACK_DIR" BUILD_DIR="$sanitized_dir" COMPARE_SANITIZE=1 \
        "$sanitized_dir/orbits-compare"
mkdir -p "$output_dir/sanitized"
if "$sanitized_dir/orbits-compare" "$output_dir/sanitized" "$filter" "${presets[@]}" &&
        "$sanitized_dir/orbits-compare" "$output_dir/sanitized" "$filter" --invalid-patches; then
        echo "No errors."
else
        echo "ERRORS"
        failures=$((failures + 1))
fi

echo
if [ "$failures" -gt 0 ]; then
        echo "$failures failures comparing with $base. The logs are in $output_dir."
        exit 1
fi
echo "No unexpected differences from $base."
