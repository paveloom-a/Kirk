project(
  'groovy',
  'c',
  version: '0.1.0',
  license: 'GPL-3.0-or-later',
  license_files: 'LICENSE.md',
  meson_version: '>=1.1.0',
  default_options: [
    'buildtype=debug',
    'c_std=c2x',
  ])

gnome = import('gnome')
i18n = import('i18n')

app_name = 'Groovy'
app_domain = 'groovy'
app_id = 'paveloom.apps.' + app_name
app_path = '/paveloom/apps/' + app_name + '/'
app_version = meson.project_version()

conf = configuration_data()
conf.set('app_name', app_name)
conf.set('app_domain', app_domain)
conf.set('app_id', app_id)
conf.set('app_path', app_path)
conf.set('app_version', app_version)

if get_option('buildtype') == 'release'
  cc_flags = []
  override_options = [
    'b_lto=true',
    'strip=true',
    'warning_level=0',
  ]
elif get_option('buildtype') == 'debug'
  app_name += ' (Development)'
  app_id += '.Devel'
  app_version += '-' + run_command(
    'git', 'rev-parse', '--short', 'HEAD',
    check: true,
  ).stdout().strip()
  cc_flags = [
    '-g3',
    '-U_FORTIFY_SOURCE',
    '-Wconversion',
    '-Wdouble-promotion',
    '-Wno-sign-conversion',
    '-Wno-unused-parameter',
    '-Wshadow',
  ]
  override_options = [
    'b_sanitize=address,undefined',
    'warning_level=3',
  ]
endif

dependencies = [
  dependency('gtk4', version: '>=4.10.4', include_type: 'system'),
]

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments(cc_flags), language: 'c')
add_project_dependencies(dependencies, language: 'c')

subdir('data')
subdir('po')

sources = files(
  'src/main.c',
)

executable = executable(
  'groovy',
  sources,
  resources,
  override_options: override_options,
  install: true,
)

if get_option('buildtype') == 'debug'
  find_program('clang-check')
  find_program('cppcheck')
  find_program('cpplint')
  find_program('desktop-file-validate')
  find_program('appstream-util')
  custom_target('lint',
      depends: executable,
      output: 'lint',
      command: 'lint.bash',
      env: {
        'APP_ID': app_id,
        'MESON_CURRENT_BUILD_DIR': meson.current_build_dir(),
        'MESON_PROJECT_SOURCE_ROOT': meson.project_source_root(),
      },
      console: true,
      build_always_stale: true,
      build_by_default: true,
  )
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  glib_compile_schemas: true,
  update_desktop_database: true
)
