ui_files = custom_target(
  'blueprints',
  input: files(
    'ui/main.blp',
  ),
  output: '.',
  command: [
    find_program('blueprint-compiler'),
    'batch-compile',
    '@OUTPUT@',
    '@CURRENT_SOURCE_DIR@',
    '@INPUT@',
  ],
)

resources = gnome.compile_resources(
  'resources',
  'groovy.gresource.xml',
  c_name: 'groovy',
  dependencies: ui_files,
)
