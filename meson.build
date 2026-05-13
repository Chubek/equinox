project('equinox', 'c',
  version: '1.0.0',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'werror=false',
  ],
  license: 'MIT',
  meson_version: '>=0.56.0'
)

eqx_build_tests = get_option('build_tests')
eqx_build_docs = get_option('build_docs')

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments(['-Wall', '-Wextra', '-pedantic']), language: 'c')

inc = include_directories(
  'include',
  'third_party',
  'third_party/klib',
  'third_party/uthash/src',
  'third_party/tlsf',
  'third_party/libdag-ethash',
)

tlsf_lib = static_library('tlsf', 'third_party/tlsf/tlsf.c', include_directories: include_directories('third_party/tlsf'), install: false)
tlsf_dep = declare_dependency(link_with: tlsf_lib, include_directories: include_directories('third_party/tlsf'))

equinox_sources = files(
  'third_party/klib/kalloc.c',
  'src/equinox/enode.c',
  'src/equinox/eclass.c',
  'src/equinox/egraph.c',
  'src/equinox/unionfind.c',
  'src/equinox/hashcons.c',
  'src/equinox/rewrite.c',
  'src/redvisage/node.c',
  'src/redvisage/lower.c',
  'src/redvisage/rewrite.c',
  'src/redvisage/extract.c',
  'src/common/memory/memory.c',
  'src/common/eqvsg/eqvsg.c',
  'src/common/dump/dump.c',
  'src/common/erl/erl.c',
  'src/common/compiler/compiler.c',
)

equinox_lib = library('equinox',
  equinox_sources,
  include_directories: inc,
  dependencies: [tlsf_dep],
  version: meson.project_version(),
  soversion: '1',
  install: true,
)

equinox_dep = declare_dependency(link_with: equinox_lib, include_directories: inc, dependencies: [tlsf_dep])

install_headers(
  'include/equinox/equinox.h',
  'include/equinox/enode.h',
  'include/equinox/eclass.h',
  'include/equinox/egraph.h',
  'include/equinox/unionfind.h',
  'include/equinox/hashcons.h',
  'include/equinox/rewrite.h',
  'include/equinox/libutils.h',
  'include/redvisage/node.h',
  'include/redvisage/lower.h',
  'include/redvisage/rewrite.h',
  'include/redvisage/extract.h',
  'include/redvisage/opcodes.h',
  subdir: 'equinox'
)

install_headers(
  'include/common/memory.h',
  'include/common/eqvsg.h',
  'include/common/dump.h',
  'include/common/erl.h',
  'include/common/compiler.h',
  subdir: 'common'
)

eqvsg_bin = executable('eqvsg', 'src/common/eqvsg/main.c', include_directories: inc, link_with: equinox_lib, install: true)

pkg = import('pkgconfig')
pkg.generate(equinox_lib, name: 'equinox', description: 'Equinox E-graph and RedVisage frontend', subdirs: 'equinox', version: meson.project_version())

if eqx_build_tests
  subdir('tests')
endif
if eqx_build_docs
  subdir('docs')
endif
