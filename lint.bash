#!/usr/bin/env bash

ROOT=$(dirname "$0");
SRC=$ROOT/src

mapfile -t SOURCES < <(
  find \
    "$SRC" \
    -type f \
    -name "*.c" -o -name "*.h")

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
