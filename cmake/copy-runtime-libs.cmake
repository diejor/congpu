# Set RUNTIME_LIBS as a global variable (make sure it's available across all CMake files)
set(RUNTIME_LIBS "" CACHE STRING "List of runtime libraries to be copied")

function(collect_runtime_libs)
  if(CONAN_RUNTIME_LIB_DIRS)
    set(resolved_dirs "")
    foreach(dir IN LISTS CONAN_RUNTIME_LIB_DIRS)
      string(REGEX REPLACE "^\\$<\\$<CONFIG:[^>]+>:" "" rdir "${dir}")
      string(REGEX REPLACE ">$" "" rdir "${rdir}")
      list(APPEND resolved_dirs "${rdir}")
    endforeach()

    foreach(dir IN LISTS resolved_dirs)
      file(GLOB libs_to_install
        "${dir}/*.dll"    # Windows
        "${dir}/*.so"     # Linux
        "${dir}/*.dylib"  # macOS
      )
      list(APPEND RUNTIME_LIBS ${libs_to_install})
    endforeach()

    message(STATUS "RUNTIME_LIBS: ${RUNTIME_LIBS}") 
  else()
    message(WARNING "No CONAN_RUNTIME_LIB_DIRS found.")
  endif()

  set(RUNTIME_LIBS ${RUNTIME_LIBS} PARENT_SCOPE)
endfunction()

collect_runtime_libs()

function(copy_runtime_libs target)
    if(RUNTIME_LIBS)
        foreach(lib IN LISTS RUNTIME_LIBS)
            add_custom_command(TARGET ${target} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${lib}"
                "$<TARGET_FILE_DIR:${target}>"
                COMMENT "Copying ${lib} to $<TARGET_FILE_DIR:${target}>"
            )
        endforeach()
    else()
        message(WARNING "RUNTIME_LIBS is empty. No libraries to copy.")
    endif()
endfunction()
