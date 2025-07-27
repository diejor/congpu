cmake_minimum_required(VERSION 3.14)

# option(TRACY_ENABLE "Enable Tracy profiler" ON)
# option(TRACY_ON_DEMAND "Enable Tracy profiler on demand" ON)

include(cmake/prelude.cmake)
include(cmake/ensure-conan.cmake)

# ---- Conan integration ----

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(_conan_toolchain "${CMAKE_SOURCE_DIR}/conan/conan_toolchain.cmake")
  if(NOT EXISTS "${_conan_toolchain}")
    message(STATUS "Conan toolchain not found, running 'conan install'")
    find_program(CONAN_COMMAND conan REQUIRED)
    if(DEFINED CMAKE_BUILD_TYPE)
      set(_conan_build_type "-s" "build_type=${CMAKE_BUILD_TYPE}")
    endif()
    execute_process(
      COMMAND ${CONAN_COMMAND} install . ${_conan_build_type} -b missing
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE _conan_result
    )
    if(NOT _conan_result EQUAL 0)
      message(FATAL_ERROR "Conan install failed: ${_conan_result}")
    endif()
  endif()
  set(CMAKE_TOOLCHAIN_FILE "${_conan_toolchain}" CACHE FILEPATH "Conan toolchain" FORCE)
endif()

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
