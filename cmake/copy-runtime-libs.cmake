function(copy_runtime_libs Target)
  if(CONAN_RUNTIME_LIB_DIRS)
    set(resolved_dirs "")
    # Resolve the actual directory paths by stripping unwanted generator expressions,
    # e.g. "$<$<CONFIG:Release>:/some/path>" or "$<$<CONFIG:Debug>:/another/path>".
    foreach(dir IN LISTS CONAN_RUNTIME_LIB_DIRS)
      # Remove the beginning "$<$<CONFIG:...>:" part, if present.
      string(REGEX REPLACE "^\\$<\\$<CONFIG:[^>]+>:" "" rdir "${dir}")
      # Remove the trailing ">", if present.
      string(REGEX REPLACE ">$" "" rdir "${rdir}")
      list(APPEND resolved_dirs "${rdir}")
    endforeach()
    message(STATUS "Resolved runtime library directories: ${resolved_dirs}")

    # For each resolved directory, collect shared library files.
    foreach(dir IN LISTS resolved_dirs)
      file(GLOB runtime_libs
        "${dir}/*.dll"    # Windows
        "${dir}/*.so"     # Linux
        "${dir}/*.dylib"  # macOS
      )
      message(STATUS "Found libraries in ${dir}: ${runtime_libs}")
      # For each library found, add a post-build command to copy it to the target's output directory.
      foreach(lib IN LISTS runtime_libs)
        add_custom_command(TARGET ${Target} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E echo "Post Build: Copying ${lib} to $<TARGET_FILE_DIR:${Target}>"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${lib}"
            "$<TARGET_FILE_DIR:${Target}>"
          COMMENT "Copying ${lib} to output directory"
        )
      endforeach()
    endforeach()
  endif()
endfunction()
