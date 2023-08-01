project(
  'kirk',
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

APP_NAME = 'Kirk'
APP_DOMAIN = 'kirk'
APP_ID = 'paveloom.apps.' + APP_NAME
APP_RESOURCES_PATH = '/paveloom/apps/' + APP_NAME + '/'
APP_VERSION = meson.project_version()
APP_DEBUG_MODE_ON = get_option('buildtype') == 'debug'

if get_option('buildtype') == 'release'
  cc_flags = []
  override_options = [
    'b_lto=true',
    'strip=true',
    'warning_level=0',
  ]
elif get_option('buildtype') == 'debug'
  cc_flags = [
    '-ferror-limit=500',
    '-g3',
    '-U_FORTIFY_SOURCE',
    '-Wconversion',
    '-Wdouble-promotion',
    '-Wno-sign-conversion',
    '-Wno-unused-parameter',
    '-Wshadow',
  ]
  override_options = [
    'warning_level=3',
  ]
endif

conf = configuration_data()
conf.set('APP_NAME', APP_NAME)
conf.set('APP_DOMAIN', APP_DOMAIN)
conf.set('APP_ID', APP_ID)
conf.set('APP_RESOURCES_PATH', APP_RESOURCES_PATH)
conf.set('APP_VERSION', APP_VERSION)
conf.set10('APP_DEBUG_MODE_ON', APP_DEBUG_MODE_ON)

dependencies = [
  dependency('libadwaita-1', version: '>=1.3.3', include_type: 'system'),
  dependency('libsecret-1', version: '>=0.20.5', include_type: 'system'),
]

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments(cc_flags), language: 'c')
add_project_dependencies(dependencies, language: 'c')

subdir('data')
subdir('include')
subdir('po')

sources = files(
  'src/kirk-add-release-window.c',
  'src/kirk-application-window.c',
  'src/kirk-application.c',
  'src/kirk-preferences-window.c',
  'src/kirk-secret-schema.c',
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
      build_always_stale: true,
  )
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  glib_compile_schemas: true,
  update_desktop_database: true
)
