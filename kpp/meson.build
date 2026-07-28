project(
  'kpp',
  'cpp',
  default_options: [
    'cpp_std=c++26'
  ]
)

sources = [
  'src/main.cpp',
  'src/lexer.cpp',
  'src/parser.cpp',
  'src/token.cpp',
  'src/interpreter.cpp',
  'src/function.cpp',
  'src/object.cpp',
]


inc_dir = include_directories('include')

libffi_dep = dependency('libffi', static: true, required: true)

executable(
  'kpp',
  sources,
  include_directories: inc_dir,
  install: true,
  dependencies: libffi_dep
)
