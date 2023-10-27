project(
  'kirk',
  'cpp',
  version: run_command(
    'grep', '-Po', '(?<=version = ").*(?=")', 'flake.nix',
    check: true,
  ).stdout().strip(),
  license: 'GPL-3.0-or-later',
  license_files: 'LICENSE.md',
  meson_version: '>=1.2.0',
  default_options: [
    'buildtype=debug',
    'cpp_std=c++2a',
  ])

gnome = import('gnome')
i18n = import('i18n')

APP_NAME = 'Kirk'
APP_DOMAIN = 'kirk'
APP_ID = 'paveloom.apps.' + APP_NAME
APP_RESOURCES_PATH = '/paveloom/apps/' + APP_NAME + '/'
APP_VERSION = meson.project_version()
APP_DEBUG_MODE_ON = get_option('buildtype') == 'debug'

conf = configuration_data()
conf.set('APP_NAME', APP_NAME)
conf.set('APP_DOMAIN', APP_DOMAIN)
conf.set('APP_ID', APP_ID)
conf.set('APP_RESOURCES_PATH', APP_RESOURCES_PATH)
conf.set('APP_VERSION', APP_VERSION)
conf.set10('APP_DEBUG_MODE_ON', APP_DEBUG_MODE_ON)

cpp_flags = []
cpp_link_flags = ['-stdlib=libc++']
override_options = []

if get_option('buildtype') == 'release'
  override_options += [
    # Use the LLD linker for LTO. In Nixpkgs, the LLVM gold plugin is disabled
    # for LLVM 16 because of https://github.com/llvm/llvm-project/issues/61350
    'b_lto=true',
    'strip=true',
    'warning_level=0',
  ]
elif get_option('buildtype') == 'debug'
  cpp_flags += [
    '-fdebug-macro',
    '-ferror-limit=500',
    '-g3',
    '-gdwarf-5',
    '-Wconversion',
    '-Wdouble-promotion',
    '-Wno-sign-conversion',
    '-Wno-unused-parameter',
    '-Wshadow',
  ]
  override_options += [
    'warning_level=3',
  ]
endif

compiler = meson.get_compiler('cpp')
add_project_arguments(compiler.get_supported_arguments(cpp_flags), language: 'cpp')
add_project_link_arguments(compiler.get_supported_link_arguments(cpp_link_flags), language : 'cpp')

dependencies = [
  dependency('glib-2.0', version: '>=2.76.4', include_type: 'system'),
  dependency('gtk4', version: '>=4.10.4', include_type: 'system'),
  dependency('libadwaita-1', version: '>=1.3.5', include_type: 'system'),
  dependency('libsecret-1', version: '>=0.20.5', include_type: 'system'),
  dependency('libsoup-3.0', version: '>=3.4.2', include_type: 'system'),
]

add_project_dependencies(dependencies, language: 'cpp')

subdir('data')
subdir('include')
subdir('po')

sources = files(
  'src/kirk-add-release-window.cpp',
  'src/kirk-application-window.cpp',
  'src/kirk-application.cpp',
  'src/kirk-preferences-window.cpp',
  'src/kirk-qobuz-client.cpp',
  'src/kirk-secret-schema.cpp',
  'src/main.cpp',
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
  find_program('clang-tidy')
  find_program('scan-build')
  find_program('pvs-studio-analyzer')
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
