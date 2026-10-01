#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

export CXX="${CXX:-clang++}"
export CC="${CC:-clang}"

# Install cpp-httplib and nlohmann/json into third_party/ the first time.
if [[ ! -d "${ROOT_DIR}/third_party/install" ]]; then
  "${ROOT_DIR}/scripts/install_deps.sh"
fi

mkdir -p "${BUILD_DIR}"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_CXX_COMPILER="${CXX}" \
  -DCMAKE_C_COMPILER="${CC}" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build "${BUILD_DIR}" --parallel

echo "Build succeeded: ${BUILD_DIR}/github_crawler"
