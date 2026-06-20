cmake_minimum_required(VERSION 3.22)

project(kilate LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build shared library (.so)" ON)
option(INSTALL "Install libkilate and executable." OFF)
option(ASAN "Enable Address Sanitizer" OFF)

add_compile_options(-Wall -Wextra -Wunused)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

set(LIB_SOURCES
    src/config.c
    src/debug.c
    src/environment.c
    src/error.c
    src/file.c
    src/interpreter.c
    src/hashmap.c
    src/lexer.c
    src/node.c
    src/parser.c
    src/native.c
    src/string.c
    src/vector.c)

# -------------------------
# LIBRARY
# -------------------------

if(BUILD_SHARED_LIBS)
  add_library(kilate_shared SHARED ${LIB_SOURCES})

  set_target_properties(kilate_shared PROPERTIES
    OUTPUT_NAME kilate
  )

  target_include_directories(kilate_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  add_library(Kilate::Shared ALIAS kilate_shared)
endif()

# -------------------------
# EXECUTABLE (IMPORTANT FIX)
# -------------------------

add_executable(kilate
    src/main.c
)

target_link_libraries(kilate
  PRIVATE kilate_shared
)

if(ASAN)
  target_compile_options(kilate_shared PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer)

  target_link_options(kilate_shared PRIVATE
    -fsanitize=address)

  target_compile_options(kilate PRIVATE
    -fsanitize=address
    -fno-omit-frame-pointer)

  target_link_options(kilate PRIVATE
    -fsanitize=address)
endif()

# -------------------------
# DEBUG
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(kilate_shared PRIVATE DEBUG)
  message(STATUS "Debug Build")
endif()

# -------------------------
# INSTALL
# -------------------------

if(INSTALL)
  include(GNUInstallDirs)

  install(TARGETS kilate
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  if(BUILD_SHARED_LIBS)
    install(TARGETS kilate_shared
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
  endif()

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()