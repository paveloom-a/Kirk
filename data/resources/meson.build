subdir('gtk')

resources = gnome.compile_resources(
  'resources',
  'groovy.gresource.xml',
  c_name: 'groovy',
  dependencies: ui_files_target,
  source_dir: meson.current_build_dir(),
)
