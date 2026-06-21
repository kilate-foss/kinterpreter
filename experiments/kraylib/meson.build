project('kraylib',
  'cpp',
  default_options: [
    'cpp_std=c++26'
  ]
)

sources = [
  'src/main.cpp',
  'src/node.cpp'
]

raylib_dep = dependency('raylib')
kilate_dep = dependency('kilate')

inc_dir = include_directories('include')
kraylib_lib = shared_library(
  'kraylib',
  sources,
  include_directories: inc_dir,
  install: true,
  install_dir: join_paths(get_option('prefix'), 'kilate/native_libs'),
  dependencies: [
    raylib_dep,
    kilate_dep
  ]
)
