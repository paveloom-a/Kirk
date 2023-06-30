blp_files = [
  'main.blp',
]

ui_files = []
foreach blp_file : blp_files
  ui_files += blp_file.replace('.blp', '.ui')
endforeach

ui_files_target = custom_target(
  'blueprints',
  input: files(blp_files),
  output: ui_files,
  command: [
    find_program('blueprint-compiler'),
    'batch-compile',
    '@OUTDIR@',
    '@CURRENT_SOURCE_DIR@',
    '@INPUT@',
  ],
)
