#!/usr/bin/env bash

mapfile -t SOURCES < \
  <(find "$MESON_PROJECT_SOURCE_ROOT/src" -type f -name "*.c" -o -name "*.h")

clang-check --analyze "${SOURCES[@]}"

cppcheck \
  --enable=all \
  --inconclusive \
  --platform=unix64 \
  --project="$MESON_CURRENT_BUILD_DIR/compile_commands.json" \
  --quiet \
  --suppress=missingInclude \
  --verbose

cpplint --verbose=0 --quiet "${SOURCES[@]}"

true
