subdir('gtk')

resources = gnome.compile_resources(
  'resources',
  'kirk.gresource.xml',
  c_name: 'kirk',
  dependencies: ui_files_target,
  source_dir: meson.current_build_dir(),
)
