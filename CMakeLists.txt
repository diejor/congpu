cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    congpu
    VERSION 0.1.0
    DESCRIPTION "Deep Neural Network Library with Dawn native"
    HOMEPAGE_URL "https://diejor.tech"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    congpu_lib OBJECT
    source/lib.cpp
    source/slang_compiler.cpp
)

target_include_directories(
    congpu_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(congpu_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(slang REQUIRED)

target_link_libraries(congpu_lib PRIVATE fmt::fmt slang::slang)

# ---- Declare executable ----

add_executable(congpu_exe source/main.cpp)
add_executable(congpu::exe ALIAS congpu_exe)

set_property(TARGET congpu_exe PROPERTY OUTPUT_NAME congpu)

target_compile_features(congpu_exe PRIVATE cxx_std_20)

find_package(Dawn REQUIRED)

target_link_libraries(congpu_exe PRIVATE
    congpu_lib
    dawn::webgpu_dawn
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT congpu_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of congpu"
  )
endif()

include(cmake/dev-mode.cmake)
