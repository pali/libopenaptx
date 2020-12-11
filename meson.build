project('openaptx', 'c',
  version: run_command('get-version.py', check: true).stdout().strip(),
  default_options: ['buildtype=debugoptimized'])

version_arr = meson.project_version().split('.')
version_major = version_arr[0].to_int()
version_minor = version_arr[1].to_int()
version_micro = version_arr[2].to_int()

soversion = version_major
libversion = '@0@.@1@.0'.format(version_major, version_minor, version_micro)
osxversion = version_minor + 1

cc = meson.get_compiler('c')

add_project_arguments('-DHAVE_CONFIG_H', language: 'c')

cdata = configuration_data()

# Symbol visibility
if cc.get_id() == 'msvc'
  export_define = '__declspec(dllexport) extern'
elif cc.has_argument('-fvisibility=hidden')
  add_project_arguments('-fvisibility=hidden', language: 'c')
  export_define = 'extern __attribute__ ((visibility ("default")))'
else
  export_define = 'extern'
endif

# Passing this through the command line would be too messy
cdata.set('APTX_API_EXPORT', export_define)

static_cflags = []
if get_option('default_library') == 'static'
  static_cflags += ['-DAPTX_STATIC_COMPILATION']
endif

# library
libopenaptx = library('openaptx', 'openaptx.c',
   c_args: ['-DBUILDING_APTX'],
   version: libversion,
   soversion: soversion,
   darwin_versions: osxversion)

openaptx_incdir = include_directories('.')

openaptx_dep = declare_dependency(link_with: libopenaptx,
  include_directories: openaptx_incdir,
  compile_args: static_cflags)

meson.override_dependency('openaptx', openaptx_dep)

# config.h
configure_file(output: 'config.h', configuration: cdata)

# pkg-config file
pkg = import('pkgconfig')
pkg.generate(libopenaptx,
  name: 'libopenaptx',
  description: 'Open Source aptX codec library',
  extra_cflags: static_cflags)

# tools
if not get_option('tools').disabled()
  executable('openaptxdec', 'openaptxdec.c', dependencies: openaptx_dep)
  executable('openaptxenc', 'openaptxenc.c', dependencies: openaptx_dep)
endif
