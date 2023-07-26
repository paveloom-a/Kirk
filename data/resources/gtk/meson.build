blp_file_names = [
  'kirk-application-window.blp',
  'kirk-preferences-window.blp',
]

blp_files = []
foreach blp_file_name : blp_file_names
  blp_files += meson.project_source_root() / 'src' / blp_file_name
endforeach

ui_files = []
foreach blp_file_name : blp_file_names
  ui_files += blp_file_name.replace('.blp', '.ui')
endforeach

ui_files_target = custom_target(
  'blueprints',
  input: files(blp_files),
  output: ui_files,
  command: [
    find_program('blueprint-compiler'),
    'batch-compile',
    '@OUTDIR@',
    meson.project_source_root() / 'src',
    '@INPUT@',
  ],
)
