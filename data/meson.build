desktop_file = i18n.merge_file(
  input: configure_file(
    input: files('paveloom.apps.Groovy.desktop.in.in'),
    output: f'@app_id@.desktop.in',
    configuration: conf,
  ),
  output: f'@app_id@.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: get_option('datadir') / 'applications'
)

appstream_file = i18n.merge_file(
  input:
  configure_file(
    input: files('paveloom.apps.Groovy.metainfo.xml.in.in'),
    output: f'@app_id@.metainfo.xml.in',
    configuration: conf,
  ),
  output: f'@app_id@.metainfo.xml',
  po_dir: '../po',
  install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

install_data(
  configure_file(
    input: files('paveloom.apps.Groovy.gschema.xml.in'),
    output: f'@app_id@.gschema.xml',
    configuration: conf,
  ),
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas',
)

subdir('icons')
subdir('resources')
