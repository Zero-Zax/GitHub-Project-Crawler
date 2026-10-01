#!/usr/bin/env bash
# Builds and installs cpp-httplib (with OpenSSL) and nlohmann/json into
# third_party/install so find_package(... CONFIG) can locate them.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/third_party/src"
PREFIX="$ROOT/third_party/install"
mkdir -p "$SRC" "$PREFIX"

fetch() {
    local repo=$1 tag=$2 dir=$3
    [[ -d "$SRC/$dir" ]] || git clone --depth 1 --branch "$tag" "$repo" "$SRC/$dir"
}

fetch https://github.com/nlohmann/json.git v3.12.0 json
cmake -S "$SRC/json" -B "$SRC/json/build" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DJSON_BuildTests=OFF
cmake --build "$SRC/json/build"
cmake --install "$SRC/json/build"

fetch https://github.com/yhirose/cpp-httplib.git v0.58.0 cpp-httplib
cmake -S "$SRC/cpp-httplib" -B "$SRC/cpp-httplib/build" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DHTTPLIB_REQUIRE_OPENSSL=ON \
    -DHTTPLIB_USE_ZLIB_IF_AVAILABLE=OFF \
    -DHTTPLIB_USE_BROTLI_IF_AVAILABLE=OFF \
    -DHTTPLIB_USE_ZSTD_IF_AVAILABLE=OFF
cmake --build "$SRC/cpp-httplib/build"
cmake --install "$SRC/cpp-httplib/build"

echo "Dependencies installed to $PREFIX"
