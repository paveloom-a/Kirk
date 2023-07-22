desktop_file = i18n.merge_file(
  input: configure_file(
    input: files('paveloom.apps.Groovy.desktop.in.in'),
    output: f'@APP_ID@.desktop.in',
    configuration: conf,
  ),
  output: f'@APP_ID@.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: get_option('datadir') / 'applications'
)

appstream_file = i18n.merge_file(
  input:
  configure_file(
    input: files('paveloom.apps.Groovy.metainfo.xml.in.in'),
    output: f'@APP_ID@.metainfo.xml.in',
    configuration: conf,
  ),
  output: f'@APP_ID@.metainfo.xml',
  po_dir: '../po',
  install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

gsettings_schema = configure_file(
  input: files('paveloom.apps.Groovy.gschema.xml.in'),
  output: f'@APP_ID@.gschema.xml',
  configuration: conf,
)
gsettings_schemas_dir = get_option('datadir') / 'glib-2.0' / 'schemas'

glib_compile_schemas = find_program('glib-compile-schemas')
custom_target('gsettings-schemas',
    input: gsettings_schema,
    output: 'gschemas.compiled',
    command: [glib_compile_schemas, '--targetdir=@OUTDIR@', '@OUTDIR@'],
    install: true,
    install_dir: gsettings_schemas_dir,
    build_by_default: true,
)

install_data(
  gsettings_schema,
  install_dir: gsettings_schemas_dir,
)

subdir('icons')
subdir('resources')
