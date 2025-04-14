#include <catch2/catch_test_macros.hpp>

#include "lib.hpp"

TEST_CASE("Name is congpu", "[library]")
{
    auto const lib = Library {};
    REQUIRE(lib.mName == "congpu");
}
