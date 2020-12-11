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

# library
libopenaptx = library('openaptx', 'openaptx.c',
   version: libversion,
   soversion: soversion,
   darwin_versions: osxversion)

openaptx_incdir = include_directories('.')

openaptx_dep = declare_dependency(link_with: libopenaptx,
  include_directories: openaptx_incdir)

meson.override_dependency('openaptx', openaptx_dep)

# pkg-config file
pkg = import('pkgconfig')
pkg.generate(libopenaptx,
  name: 'libopenaptx',
  description: 'Open Source aptX codec library')

# tools
if not get_option('tools').disabled()
  executable('openaptxdec', 'openaptxdec.c', dependencies: openaptx_dep)
  executable('openaptxenc', 'openaptxenc.c', dependencies: openaptx_dep)
endif
