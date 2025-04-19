#pragma once

#include <cstdint>    // uint32_t
#include <cstdio>    // std::FILE*
#include <source_location>    // std::source_location::current()

#include <fmt/format.h>
#include <fmt/ostream.h>    // for operator<< fall‑back

#ifdef TRACY_ENABLE
#    include <tracy/Tracy.hpp>
#endif

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

enum class LogLevel
{
    Error = 0,
    Warn,
    Info,
    Trace
};
inline constexpr LogLevel kMinCompileTimeLevel = LogLevel::Trace;

inline constexpr const char* LOG_COLORS[] = {
    "\033[0;31m",    // Error
    "\033[0;33m",    // Warn
    "\033[0;37m",    // Info
    "\033[0;90m"    // Trace
};
inline constexpr const char* RESET_COLOR = "\033[0m";

inline constexpr std::string_view level_name(LogLevel L) noexcept
{
    switch (L) {
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Trace:
            return "TRACE";
    }
    return "UNKNOWN";
}

#ifdef TRACY_ENABLE
template<LogLevel L>
inline void tracy_emit(const char* txt, size_t sz)
{
    constexpr uint32_t cols[] = {0xFF0000u, 0xFFAA00u, 0xFFFFFFu, 0xAAAAAAu};
    tracy::Profiler::MessageColor(txt, sz, cols[static_cast<int>(L)], 0);
}
#else
template<LogLevel>
inline constexpr void tracy_emit(const char*, size_t) noexcept
{
}
#endif

namespace detail
{
template<LogLevel L, typename... Args>
inline void sink(std::FILE* stream,
                 std::string_view fmtStr,
                 const std::source_location& loc,
                 Args&&... args)
{
    if constexpr (L > kMinCompileTimeLevel) {
        return;
    }
    auto msg = fmt::vformat(fmtStr, fmt::make_format_args(args...));
    tracy_emit<L>(msg.c_str(), msg.size());
    fmt::print(stream,
               "{}[{}:{} {}()][{}]{} {}\n",
               LOG_COLORS[static_cast<int>(L)],
               loc.file_name(),
               loc.line(),
               loc.function_name(),
               level_name(L),
               RESET_COLOR,
               msg);
}
}    // namespace detail

#ifdef NO_LOG
#    define LOG_ERROR(...) ((void)0)
#    define LOG_WARN(...) ((void)0)
#    define LOG_INFO(...) ((void)0)
#    define LOG_TRACE(...) ((void)0)
#else
#    define LOG_ERROR(fmt, ...) \
        ::detail::sink<LogLevel::Error>(stdout, \
                                        fmt, \
                                        std::source_location::current() \
                                            __VA_OPT__(, __VA_ARGS__))
#    define LOG_WARN(fmt, ...) \
        ::detail::sink<LogLevel::Warn>(stdout, \
                                       fmt, \
                                       std::source_location::current() \
                                           __VA_OPT__(, __VA_ARGS__))
#    define LOG_INFO(fmt, ...) \
        ::detail::sink<LogLevel::Info>(stdout, \
                                       fmt, \
                                       std::source_location::current() \
                                           __VA_OPT__(, __VA_ARGS__))
#    define LOG_TRACE(fmt, ...) \
        ::detail::sink<LogLevel::Trace>(stdout, \
                                        fmt, \
                                        std::source_location::current() \
                                            __VA_OPT__(, __VA_ARGS__))
#endif

namespace fmt
{

template<>
struct formatter<wgpu::StringView> : formatter<std::string_view>
{
    constexpr auto parse(format_parse_context& ctx)
    {
        return formatter<std::string_view>::parse(ctx);
    }

    auto format(wgpu::StringView const& sv, format_context& ctx) const
    {
        return formatter<std::string_view>::format(
            static_cast<std::string_view>(sv), ctx);
    }
};

template<typename T>
    requires std::is_enum_v<T>
struct formatter<T> : ostream_formatter
{
};

}    // namespace fmt
