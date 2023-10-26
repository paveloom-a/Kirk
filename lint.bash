#!/usr/bin/env bash

mapfile -t SOURCES < <(
  find \
    "$MESON_PROJECT_SOURCE_ROOT/src" \
    -type f \
    -name "*.c" -o -name "*.cpp" -o -name "*.h")

appstream-util validate \
  "$MESON_CURRENT_BUILD_DIR/data/$APP_ID.metainfo.xml"

clang-check --analyze "${SOURCES[@]}"

cppcheck \
  --check-level=exhaustive \
  --enable=all \
  --inconclusive \
  --library=gtk \
  --quiet \
  --suppressions-list="$MESON_PROJECT_SOURCE_ROOT"/.cppcheck \
  --verbose \
  -I "$MESON_CURRENT_BUILD_DIR" \
  -I "$MESON_PROJECT_SOURCE_ROOT" \
  "${SOURCES[@]}"

cpplint --verbose=0 --quiet "${SOURCES[@]}"

clang-tidy --quiet "${SOURCES[@]}"

ninja -C "$MESON_CURRENT_BUILD_DIR" scan-build

pvs-studio-analyzer analyze \
  --analysis-mode '64;GA;OP;CS;MISRA;AUTOSAR;OWASP' \
  --exclude-path "$MESON_CURRENT_BUILD_DIR/data/resources" \
  --exclude-path "/nix/store/*" \
  --file "$MESON_CURRENT_BUILD_DIR/compile_commands.json" \
  --output-file "$MESON_CURRENT_BUILD_DIR/PVS-Studio.log" \
  --rules-config "$MESON_PROJECT_SOURCE_ROOT/.pvsconfig"
rm -rf "$MESON_CURRENT_BUILD_DIR/pvs-studio"
plog-converter \
  --analyzer '64:1,2,3;GA:1,2,3;OP:1,2,3;CS:1,2,3;MISRA:1,2,3;AUTOSAR:1,2,3;OWASP:1,2,3' \
  --output "$MESON_CURRENT_BUILD_DIR/pvs-studio" \
  --renderTypes fullhtml \
  "$MESON_CURRENT_BUILD_DIR/PVS-Studio.log"

desktop-file-validate "$MESON_CURRENT_BUILD_DIR/data/$APP_ID.desktop"

glib-compile-schemas \
  --strict \
  --dry-run \
  "$MESON_CURRENT_BUILD_DIR/data"

while read -r potfile; do
  if [ ! -f "$MESON_PROJECT_SOURCE_ROOT/$potfile" ]; then
    echo "No such file: $MESON_PROJECT_SOURCE_ROOT/$potfile"
  fi
done < "$MESON_PROJECT_SOURCE_ROOT/po/POTFILES"
