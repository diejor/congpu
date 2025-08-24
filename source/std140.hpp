#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace std140
{

class Encoder
{
  public:
    class StructScope
    {
      public:
        explicit StructScope(Encoder& e)
            : encoder(e)
            , start(e.startStruct())
        {
        }

        ~StructScope() { encoder.endStruct(start); }

        StructScope(const StructScope&) = delete;
        StructScope& operator=(const StructScope&) = delete;

      private:
        Encoder& encoder;
        std::size_t start;
    };

    template<typename T>
    void write(const T& value)
    {
        constexpr std::size_t alignment = baseAlignment<T>();
        align(alignment);
        const std::byte* data = reinterpret_cast<const std::byte*>(&value);
        buffer.insert(buffer.end(), data, data + sizeof(T));
    }

    [[nodiscard]] StructScope beginStruct() { return StructScope(*this); }

    [[nodiscard]] const std::vector<std::byte>& data() const { return buffer; }

  private:
    std::vector<std::byte> buffer;

    std::size_t startStruct()
    {
        align(16);
        return buffer.size();
    }

    void endStruct(std::size_t /*start*/) { align(16); }

    void align(std::size_t alignment)
    {
        std::size_t aligned =
            (buffer.size() + alignment - 1) & ~(alignment - 1);
        if (aligned > buffer.size()) {
            buffer.resize(aligned, std::byte {0});
        }
    }

    template<typename T>
    static consteval std::size_t baseAlignment()
    {
        if constexpr (std::is_same_v<T, float>
                      || std::is_same_v<T, std::int32_t>
                      || std::is_same_v<T, std::uint32_t>)
        {
            return 4;
        } else {
            return alignof(T);
        }
    }
};

}    // namespace std140
