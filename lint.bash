#!/usr/bin/env bash

mapfile -t SOURCES < \
  <(find \
      "$MESON_PROJECT_SOURCE_ROOT/src" \
      -type f \
      -name "*.c" -o -name "*.h" \
      -not -name "groovy-resources.*")

clang-check --analyze "${SOURCES[@]}"

cppcheck \
  --enable=all \
  --inconclusive \
  --platform=unix64 \
  --quiet \
  --suppress=missingInclude \
  --verbose \
  "${SOURCES[@]}"

cpplint --verbose=0 --quiet "${SOURCES[@]}"

true
