#!/bin/bash
# Build the Affinity on Linux loader and package it for release.
#
#   ./build.sh            build into dist/ and make dist/aol-loader-<version>.tar.xz
#
# Needs the .NET SDK and the MinGW-w64 cross compiler. The output is
# reproducible: paths are mapped, timestamps and owners fixed, so two builds of
# the same commit give the same tarball and the installer's SHA-256 pin holds.
set -euo pipefail

ROOT=$(dirname "$(readlink -f "$0")")
cd "$ROOT"
VERSION=$(grep -oP '<Version>\K[^<]+' Loader/AolLoader.csproj | head -1)
OUT="$ROOT/dist/aol-loader"
EPOCH=$(git log -1 --format=%ct 2>/dev/null || echo 0)

rm -rf "$ROOT/dist"
mkdir -p "$OUT/aol/plugins"

dotnet build Fixes/AolFixes.csproj -c Release -nologo \
    -p:Deterministic=true -p:ContinuousIntegrationBuild=true \
    -p:PathMap="$ROOT=/src" -o "$ROOT/dist/bin"

x86_64-w64-mingw32-gcc -O2 -s -shared -ffile-prefix-map="$ROOT=/src" \
    -Wl,--no-insert-timestamp \
    -o "$OUT/aol-bootstrap.dll" Bootstrap/aol-bootstrap.c

cp "$ROOT/dist/bin/aol-loader.dll" "$ROOT/dist/bin/0Harmony.dll" "$OUT/"
cp "$ROOT/dist/bin/aol-fixes.dll" "$OUT/aol/plugins/"
cp Loader/LICENSE "$OUT/LICENSE-aol-loader.txt"
cp Bootstrap/LICENSE "$OUT/LICENSE-aol-bootstrap.txt"
cp Fixes/LICENSE "$OUT/aol/plugins/LICENSE-aol-fixes.txt"

tar --sort=name --mtime="@$EPOCH" --owner=0 --group=0 --numeric-owner --mode=u+rw,go+r,go-w \
    -C "$ROOT/dist" -cf - aol-loader | xz -9e -T1 > "$ROOT/dist/aol-loader-$VERSION.tar.xz"
cd "$ROOT/dist"
sha256sum "aol-loader-$VERSION.tar.xz"
