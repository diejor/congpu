#include "lib.hpp"

#include <fmt/core.h>
#include <fmt/format.h>

library::library()
    : name {fmt::format("{}", "congpu")}
{
}
