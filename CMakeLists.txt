cmake_minimum_required(VERSION 3.14)

# option(TRACY_ENABLE "Enable Tracy profiler" ON)
# option(TRACY_ON_DEMAND "Enable Tracy profiler on demand" ON)

include(cmake/prelude.cmake)

project(
    congpu
    VERSION 0.1.0
    DESCRIPTION "Deep Neural Network Library with Dawn native"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/copy-runtime-libs.cmake)

# ---- Declare library ----

add_library(
    congpu_lib OBJECT
    source/lib.cpp
    source/slang_compiler.cpp
    source/shaders/tools/gpu-printing.cpp
)

target_include_directories(
    congpu_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(congpu_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
find_package(slang REQUIRED)
find_package(dawn REQUIRED)
find_package(Tracy REQUIRED)

target_link_libraries(congpu_lib PUBLIC
    fmt::fmt 
    slang::slang
    dawn::webgpu_dawn
    Tracy::TracyClient
)

# ---- Declare executable ----

add_executable(congpu_exe source/main.cpp)
add_executable(congpu::exe ALIAS congpu_exe)

copy_runtime_libs(congpu_exe)

set_property(TARGET congpu_exe PROPERTY OUTPUT_NAME congpu)

target_compile_features(congpu_exe PRIVATE cxx_std_20)

find_package(dawn REQUIRED)

target_link_libraries(congpu_exe PRIVATE
    congpu_lib
    Tracy::TracyClient
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
