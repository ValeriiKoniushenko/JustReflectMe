/*
 * This code was generated automatically with
 * https://github.com/ValeriiKoniushenko/JustReflectMe
 *
 * DO NOT EDIT MANUALLY!
 * Your changes will be replaced next time
 */

#include <array>
#include <optional>
#include <string>
#include <unordered_map>

template<>
struct R<TestEnum>
{
    static constexpr std::string_view Name() { return "TestEnum"; }
    static constexpr std::string_view FullName() { return "TestEnum"; }
    static constexpr std::size_t Size() { return 2; }
    static constexpr std::string_view ParentScope() { return ""; }

    static std::string_view ToString(::TestEnum value)
    {
        const auto& data = R<TestEnum>::ToMapCN();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        static constexpr std::string_view empty{};
        return empty;
    }

    static std::optional<::TestEnum> FromString(std::string_view value)
    {
        const auto& data = R<TestEnum>::ToMapNC();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        return std::nullopt;
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static void Serialize(::TestEnum value, RResourceStream<RImpl>& s)
    {
        s.getData() = ToString(value);
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static RResourceStream<RImpl> Serialize(::TestEnum value)
    {
        RResourceStream<RImpl> s;
        Serialize<RImpl>(value, s);
        return s;
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static void Deserialize(const RResourceStream<RImpl>& s, ::TestEnum& value)
    {
        auto tmp = FromString(s.getData().template get<std::string>());
        if (tmp.has_value())
            value = tmp.value();
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    [[nodiscard]] static ::TestEnum Deserialize(const RResourceStream<RImpl>& s)
    {
        ::TestEnum out;
        Deserialize<RImpl>(s, out);
        return out;
    }

    static constexpr const std::array<::TestEnum, 2>& ToArrayC()
    {
        static constexpr std::array<::TestEnum, 2> constants = {
				::TestEnum::Hello,
				::TestEnum::World
        };

        return constants;
    }

    static constexpr const std::array<std::string_view, 2>& ToArrayN()
    {
        static constexpr std::array<std::string_view, 2> names = {
				std::string_view("Hello"),
				std::string_view("World")
        };

        return names;
    }

    static const std::unordered_map<::TestEnum, std::string_view>& ToMapCN()
    {
        static const std::unordered_map<::TestEnum, std::string_view> map = {
				{ ::TestEnum::Hello, "Hello" },
				{ ::TestEnum::World, "World" }
        };

        return map;
    }

    static const std::unordered_map<std::string_view, ::TestEnum>& ToMapNC()
    {
        static const std::unordered_map<std::string_view, ::TestEnum> map = {
				{ "Hello", ::TestEnum::Hello },
				{ "World", ::TestEnum::World }
        };

        return map;
    }
}; // struct R<TestEnum>

