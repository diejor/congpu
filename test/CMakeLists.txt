# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(congpuTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(congpu_test 
    source/lib_test.cpp
    source/slang_test.cpp
)

copy_runtime_libs(congpu_test)

target_link_libraries(
    congpu_test PRIVATE
    congpu_lib
    Catch2::Catch2WithMain
)

target_compile_options(congpu_test PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:-U TRACY_ENABLE>"
)

target_compile_definitions(congpu_test PRIVATE
    NO_LOG
    SHADERS_DIR="${CMAKE_SOURCE_DIR}/test/source/shaders/"
)

target_compile_features(congpu_test PRIVATE cxx_std_20)

catch_discover_tests(congpu_test)

# ---- End-of-file commands ----

add_folders(Test)
