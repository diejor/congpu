# Auto-run Conan install if the toolchain file is missing

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(_conan_toolchain "${CMAKE_SOURCE_DIR}/conan/conan_toolchain.cmake")
  if(NOT EXISTS "${_conan_toolchain}")
    message(STATUS "Conan toolchain not found, running 'conan install'")
    find_program(CONAN_COMMAND conan)
    if(NOT CONAN_COMMAND)
      message(FATAL_ERROR
          "Conan executable not found. Install it or disable with -DCONGPU_USE_CONAN=OFF")
    endif()
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