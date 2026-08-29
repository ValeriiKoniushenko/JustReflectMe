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
struct R<NS::NS222::NS222_333::TestEnum>
{
    static constexpr std::string_view Name() { return "TestEnum"; }
    static constexpr std::string_view FullName() { return "NS::NS222::NS222_333::TestEnum"; }
    static constexpr std::size_t Size() { return 2; }
    static constexpr std::string_view ParentScope() { return "NS::NS222::NS222_333"; }

    static std::string_view ToString(::NS::NS222::NS222_333::TestEnum value)
    {
        const auto& data = R<NS::NS222::NS222_333::TestEnum>::ToMapCN();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        static constexpr std::string_view empty{};
        return empty;
    }

    static std::optional<::NS::NS222::NS222_333::TestEnum> FromString(std::string_view value)
    {
        const auto& data = R<NS::NS222::NS222_333::TestEnum>::ToMapNC();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        return std::nullopt;
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static void Serialize(::NS::NS222::NS222_333::TestEnum value, RResourceStream<RImpl>& s)
    {
        s.getData() = ToString(value);
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static RResourceStream<RImpl> Serialize(::NS::NS222::NS222_333::TestEnum value)
    {
        RResourceStream<RImpl> s;
        Serialize<RImpl>(value, s);
        return s;
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    static void Deserialize(const RResourceStream<RImpl>& s, ::NS::NS222::NS222_333::TestEnum& value)
    {
        auto tmp = FromString(s.getData().template get<std::string>());
        if (tmp.has_value())
            value = tmp.value();
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    [[nodiscard]] static ::NS::NS222::NS222_333::TestEnum Deserialize(const RResourceStream<RImpl>& s)
    {
        ::NS::NS222::NS222_333::TestEnum out;
        Deserialize<RImpl>(s, out);
        return out;
    }

    static constexpr const std::array<::NS::NS222::NS222_333::TestEnum, 2>& ToArrayC()
    {
        static constexpr std::array<::NS::NS222::NS222_333::TestEnum, 2> constants = {
				::NS::NS222::NS222_333::TestEnum::Hello,
				::NS::NS222::NS222_333::TestEnum::World
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

    static const std::unordered_map<::NS::NS222::NS222_333::TestEnum, std::string_view>& ToMapCN()
    {
        static const std::unordered_map<::NS::NS222::NS222_333::TestEnum, std::string_view> map = {
				{ ::NS::NS222::NS222_333::TestEnum::Hello, "Hello" },
				{ ::NS::NS222::NS222_333::TestEnum::World, "World" }
        };

        return map;
    }

    static const std::unordered_map<std::string_view, ::NS::NS222::NS222_333::TestEnum>& ToMapNC()
    {
        static const std::unordered_map<std::string_view, ::NS::NS222::NS222_333::TestEnum> map = {
				{ "Hello", ::NS::NS222::NS222_333::TestEnum::Hello },
				{ "World", ::NS::NS222::NS222_333::TestEnum::World }
        };

        return map;
    }
}; // struct R<NS::NS222::NS222_333::TestEnum>

