project(
  'groovy',
  'c',
  version: run_command(
    'grep', '-Po', '(?<=version = ").*(?=")', 'flake.nix',
    check: true,
  ).stdout().strip(),
  license: 'GPL-3.0-or-later',
  license_files: 'LICENSE.md',
  meson_version: '>=1.1.0',
  default_options: [
    'buildtype=debug',
    'c_std=c2x',
  ])

gnome = import('gnome')
i18n = import('i18n')

APP_NAME = 'Groovy'
APP_DOMAIN = 'groovy'
APP_ID = 'paveloom.apps.' + APP_NAME
APP_PATH = '/paveloom/apps/' + APP_NAME + '/'
APP_VERSION = meson.project_version()

if get_option('buildtype') == 'release'
  cc_flags = []
  override_options = [
    'b_lto=true',
    'strip=true',
    'warning_level=0',
  ]
elif get_option('buildtype') == 'debug'
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

conf = configuration_data()
conf.set('APP_NAME', APP_NAME)
conf.set('APP_DOMAIN', APP_DOMAIN)
conf.set('APP_ID', APP_ID)
conf.set('APP_PATH', APP_PATH)
conf.set('APP_VERSION', APP_VERSION)

dependencies = [
  dependency('gtk4', version: '>=4.10.4', include_type: 'system'),
]

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments(cc_flags), language: 'c')
add_project_dependencies(dependencies, language: 'c')

subdir('data')
subdir('include')
subdir('po')

sources = files(
  'src/main.c',
)

executable = executable(
  meson.project_name(),
  resources,
  sources,
  override_options: override_options,
  include_directories: [
    include_dir,
  ],
  install: true,
)

if get_option('buildtype') == 'debug'
  find_program('appstream-util')
  find_program('clang-check')
  find_program('cppcheck')
  find_program('cpplint')
  find_program('desktop-file-validate')
  find_program('glib-compile-schemas')
  custom_target('lint',
      depends: executable,
      output: 'lint',
      command: 'lint.bash',
      env: {
        'APP_ID': APP_ID,
        'MESON_CURRENT_BUILD_DIR': meson.current_build_dir(),
        'MESON_PROJECT_SOURCE_ROOT': meson.project_source_root(),
      },
      console: true,
  )
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  glib_compile_schemas: true,
  update_desktop_database: true
)
