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

dependencies = [
  dependency('gtk4', version: '>=4.10.4', include_type: 'system'),
]

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments(cc_flags), language: 'c')
add_project_dependencies(dependencies, language: 'c')

groovy_resources = gnome.compile_resources(
  'groovy-resources',
  'src/resources/groovy.gresource.xml',
  c_name: 'groovy',
  source_dir: 'src/resources'
)

groovy_sources = [
  'src/groovy-main.c',
  groovy_resources,
]

executable = executable(
  'groovy',
  groovy_sources,
  override_options: override_options,
  install: true,
)

if get_option('buildtype') == 'debug'
  clang_check = find_program('clang-check')
  cppcheck = find_program('cppcheck')
  cpplint = find_program('cpplint')
  custom_target('lint',
      depends: executable,
      output: 'lint',
      command: 'lint.bash',
      console: true,
      env: {
        'MESON_PROJECT_SOURCE_ROOT': meson.project_source_root(),
        'MESON_CURRENT_BUILD_DIR': meson.current_build_dir(),
      },
      build_always_stale: true,
      build_by_default: true,
  )
endif

gnome.post_install(
  gtk_update_icon_cache: true,
  glib_compile_schemas: true,
  update_desktop_database: true
)
