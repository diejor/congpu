#include <catch2/catch_test_macros.hpp>

#include "lib.hpp"

TEST_CASE("Name is congpu", "[library]")
{
    auto lib = Library {};
    wgpu::Instance instance = lib.CreateInstance();
    REQUIRE(lib.mName == "congpu");
}
