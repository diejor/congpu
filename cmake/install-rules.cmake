
if(UNIX AND NOT APPLE)
    set_target_properties(congpu_exe PROPERTIES INSTALL_RPATH "$ORIGIN:$ORIGIN")                       
elseif(APPLE)
    set_target_properties(congpu_exe PROPERTIES INSTALL_RPATH "@executable_path")                       
endif()

install(FILES ${RUNTIME_LIBS}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT congpu_Runtime
)

install(
    TARGETS congpu_exe
    RUNTIME COMPONENT congpu_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()

