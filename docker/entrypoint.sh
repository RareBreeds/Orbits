#!/bin/sh

set -eu

J=-j8

cd /opt/plugin
make $J clean

make $J dist
make $J clean

export CC=x86_64-apple-darwin15-clang
export CXX=x86_64-apple-darwin15-clang++
export STRIP=x86_64-apple-darwin15-strip
make $J dist
make $J clean

export CC=x86_64-w64-mingw32-gcc-posix
export CXX=x86_64-w64-mingw32-g++-posix
export STRIP=x86_64-w64-mingw32-strip
make $J dist
make $J clean
