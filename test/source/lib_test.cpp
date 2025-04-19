#include "lib.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is congpu", "[library]")
{
    auto lib = Library {};
    wgpu::Instance instance = lib.CreateInstance();
    REQUIRE(lib.mName == "congpu");
}

TEST_CASE("CreateInstance", "[library]")
{
    auto lib = Library {};
    wgpu::Instance instance = lib.CreateInstance();
    REQUIRE(instance != nullptr);
}

TEST_CASE("RequestAdapter", "[library]")
{
    auto lib = Library {};
    wgpu::Instance instance = lib.CreateInstance();
    wgpu::Adapter adapter = lib.RequestAdapter(instance);
    REQUIRE(adapter != nullptr);
}

TEST_CASE("RequestDevice", "[library]")
{
    auto lib = Library {};
    wgpu::Instance instance = lib.CreateInstance();
    wgpu::Adapter adapter = lib.RequestAdapter(instance);
    wgpu::Device device = lib.RequestDevice(adapter);
    REQUIRE(device != nullptr);
}
