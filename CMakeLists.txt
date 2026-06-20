cmake_minimum_required(VERSION 3.22)

project(kilate LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build shared library (.so)" ON)
option(BUILD_STATIC_LIBS "Build static library (.a)" ON)
option(INSTALL "Install libkilate and executable." OFF)
option(ASAN "Enable Address Sanitizer" OFF)

add_compile_options(-Wall -Wextra -Wunused)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

if(ASAN)
  message(STATUS "AddressSanitizer enabled")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

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
    src/vector.c
)

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

if(BUILD_STATIC_LIBS)
  add_library(kilate_static STATIC ${LIB_SOURCES})

  set_target_properties(kilate_static PROPERTIES
    OUTPUT_NAME kilate_static
  )

  target_include_directories(kilate_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

  add_library(Kilate::Static ALIAS kilate_static)
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

# -------------------------
# DEBUG
# -------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(kilate PRIVATE DEBUG)
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

  if(BUILD_STATIC_LIBS)
    install(TARGETS kilate_static
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
  endif()

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()