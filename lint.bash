#!/usr/bin/env bash

mapfile -t SOURCES < <(
  find \
    "$MESON_PROJECT_SOURCE_ROOT/src" \
    -type f \
    -name "*.c" -o -name "*.h")

appstream-util validate \
  "$MESON_CURRENT_BUILD_DIR/data/$APP_ID.metainfo.xml"

clang-check --analyze "${SOURCES[@]}"

cppcheck \
  --enable=all \
  --check-level=exhaustive \
  --inconclusive \
  --quiet \
  --suppress=missingIncludeSystem \
  --verbose \
  -I "$MESON_CURRENT_BUILD_DIR" \
  "${SOURCES[@]}"

cpplint --verbose=0 --quiet "${SOURCES[@]}"

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

# Generate a pseudo-output
touch "$MESON_CURRENT_BUILD_DIR/lint"
