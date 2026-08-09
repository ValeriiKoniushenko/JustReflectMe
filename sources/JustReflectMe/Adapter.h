/*
 * MIT License
 *
 * Copyright (c) 2018-2027 Valerii Koniushenko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "NlohmannJson.h"

#include <string>
#include <string_view>
#include <vector>

//
//  ┏┳┓╻┏━┓┏━╸
//  ┃┃┃┃┗━┓┃
//  ╹ ╹╹┗━┛┗━╸
//

struct RClassField
{
    std::string_view type;
    std::string_view name;
};

namespace RInternal
{
    [[nodiscard]] std::unordered_map<std::string, RClassField> GetClassFieldsAsMap(
        const std::vector<RClassField>& fields);
} // namespace RInternal

template<class T = void>
struct R
{
    constexpr static uint64_t Default = 1 << 0;
    constexpr static uint64_t Min = 1 << 1;
    constexpr static uint64_t NoDefaultValue = 1 << 2;
};

using RPoint = R<void>;

enum RFlag
{
    RFlag_Required = 1 << 1,
    RFlag_NonRequired = 1 << 2,
    RFlag_Default = RFlag_NonRequired, // Default behavior
};

enum class RStatus
{
    Ok,
    NotFound
};

inline const char* RStatusToString(RStatus status)
{
    // clang-format off
    if (status == RStatus::Ok) return "Ok";
    if (status == RStatus::NotFound) return "NotFound";
    return "Unknown";
    // clang-format on
}

using RLogsCollector = std::vector<std::pair<std::string, RStatus>>;

template<class T>
concept RHasOnPreDeserialize = requires(std::remove_cv_t<T> v) { v.onPreDeserialize(nullptr); };

template<class T>
concept RHasOnPostDeserialize
    = requires(T& v, const RLogsCollector& logs) { v.onPostDeserialize(&v, logs); };

template<class T>
concept RHasOnPreSerialize = requires(std::remove_cv_t<T> v) { v.onPreSerialize(nullptr); };

template<class T>
concept RHasOnPostSerialize = requires(std::remove_cv_t<T> v, const RLogsCollector& logs) {
    v.onPostSerialize(nullptr, logs);
};

template<typename T>
void _RTryCallPreSerialize(const T& obj)
{
    if constexpr (RHasOnPreSerialize<T>)
    {
        obj.onPreSerialize(&obj);
    }
}

template<typename T>
void _RTryCallPostSerialize(const T& obj, const RLogsCollector& logs)
{
    if constexpr (RHasOnPostSerialize<T>)
    {
        obj.onPostSerialize(&obj, logs);
    }
}

template<typename T>
void _RTryCallPreDeserialize(T& obj)
{
    if constexpr (RHasOnPreDeserialize<T>)
    {
        obj.onPreDeserialize(&obj);
    }
}

template<typename T>
void _RTryCallPostDeserialize(T& obj, const RLogsCollector& logs)
{
    if constexpr (RHasOnPostDeserialize<T>)
    {
        obj.onPostDeserialize(&obj, logs);
    }
}

//
//  ┏━┓┏┓ ┏━┓┏━┓┏━╸┏━┓┏━╸┏━┓┏━┓╻ ╻┏━┓┏━╸┏━╸┏━┓╺┳╸┏━┓┏━╸┏━┓┏┳┓╻┏┳┓┏━┓╻
//  ┣┳┛┣┻┓┣━┫┗━┓┣╸ ┣┳┛┣╸ ┗━┓┃ ┃┃ ┃┣┳┛┃  ┣╸ ┗━┓ ┃ ┣┳┛┣╸ ┣━┫┃┃┃┃┃┃┃┣━┛┃
//  ╹┗╸┗━┛╹ ╹┗━┛┗━╸╹┗╸┗━╸┗━┛┗━┛┗━┛╹┗╸┗━╸┗━╸┗━┛ ╹ ╹┗╸┗━╸╹ ╹╹ ╹╹╹ ╹╹  ┗━╸
//

template<class DataT>
class RBaseResourceStreamImpl
{
public:
    using DataType = DataT;

    RBaseResourceStreamImpl() = default;
    virtual ~RBaseResourceStreamImpl() = default;

    [[nodiscard]] DataT& data() noexcept { return _data; }
    [[nodiscard]] const DataT& data() const noexcept { return _data; }

    [[nodiscard]] const RLogsCollector& logs() const noexcept { return _logs; }
    [[nodiscard]] RLogsCollector& logs() noexcept { return _logs; }

protected:
    DataT _data;
    mutable RLogsCollector _logs;
};

template<class T>
concept DerivedFromAnyRBaseResourceStreamImpl
    = requires(std::remove_cvref_t<T>* p) { []<class U>(RBaseResourceStreamImpl<U>*) {}(p); };

template<class T>
concept IsResourceStreamImpl = requires(T t, int v) {
    requires DerivedFromAnyRBaseResourceStreamImpl<T>;
    { t.template read<int>("foo", v, RFlag_Default) } -> std::same_as<void>;
    { t.template read<int>("foo", v, 100) } -> std::same_as<void>;
    { t.template write<int>("foo", 12345) } -> std::same_as<void>;
};

//
//  ┏━┓┏━┓┏━╸┏━┓┏━┓╻ ╻┏━┓┏━╸┏━╸┏━┓╺┳╸┏━┓┏━╸┏━┓┏┳┓
//  ┣┳┛┣┳┛┣╸ ┗━┓┃ ┃┃ ┃┣┳┛┃  ┣╸ ┗━┓ ┃ ┣┳┛┣╸ ┣━┫┃┃┃
//  ╹┗╸╹┗╸┗━╸┗━┛┗━┛┗━┛╹┗╸┗━╸┗━╸┗━┛ ╹ ╹┗╸┗━╸╹ ╹╹ ╹
//

template<IsResourceStreamImpl RImpl>
struct RResourceStream
{
public:
    RResourceStream() = default;
    virtual ~RResourceStream() = default;

    RResourceStream(const typename RImpl::DataType& out) { impl.data() = out; }

    template<class T>
    void write(std::string_view fieldName, const T& value)
    {
        impl.template write<T>(fieldName, value);
    }

    template<class T>
    void write(const T& value)
    {
        impl.template write<T>(value);
    }

    template<class T>
    void read(std::string_view fieldName, T& value) const
    {
        impl.template read<T>(fieldName, value);
    }

    template<class T, class T2>
    void read(std::string_view fieldName, T& value, T2&& defaultValue) const
    {
        impl.template read<T>(fieldName, value, std::forward<decltype(defaultValue)>(defaultValue));
    }

    [[nodiscard]] const auto& getData() const noexcept { return impl.data(); }
    [[nodiscard]] auto& getData() noexcept { return impl.data(); }

    [[nodiscard]] const RLogsCollector& logs() const noexcept { return impl.logs(); }
    [[nodiscard]] RLogsCollector& logs() noexcept { return impl.logs(); }

protected:
    RImpl impl;
};

// ------------------------------------------------------------------- //
//                                                                     //
//  ╺┳┓┏━╸┏━╸┏━┓╻ ╻╻  ╺┳╸   ╻┏┳┓┏━┓╻  ┏━╸┏┳┓┏━╸┏┓╻╺┳╸┏━┓╺┳╸╻┏━┓┏┓╻┏━┓  //
//   ┃┃┣╸ ┣╸ ┣━┫┃ ┃┃   ┃    ┃┃┃┃┣━┛┃  ┣╸ ┃┃┃┣╸ ┃┗┫ ┃ ┣━┫ ┃ ┃┃ ┃┃┗┫┗━┓  //
//  ╺┻┛┗━╸╹  ╹ ╹┗━┛┗━╸ ╹    ╹╹ ╹╹  ┗━╸┗━╸╹ ╹┗━╸╹ ╹ ╹ ╹ ╹ ╹ ╹┗━┛╹ ╹┗━┛  //
//                                                                     //
// ------------------------------------------------------------------- //

//
//  ┏━┓ ┏┓┏━┓┏━┓┏┓╻┏━┓┏━╸┏━┓┏━┓╻ ╻┏━┓┏━╸┏━╸┏━┓╺┳╸┏━┓┏━╸┏━┓┏┳┓
//  ┣┳┛  ┃┗━┓┃ ┃┃┗┫┣┳┛┣╸ ┗━┓┃ ┃┃ ┃┣┳┛┃  ┣╸ ┗━┓ ┃ ┣┳┛┣╸ ┣━┫┃┃┃
//  ╹┗╸┗━┛┗━┛┗━┛╹ ╹╹┗╸┗━╸┗━┛┗━┛┗━┛╹┗╸┗━╸┗━╸┗━┛ ╹ ╹┗╸┗━╸╹ ╹╹ ╹
//

class RJsonResourceStream : public RBaseResourceStreamImpl<nlohmann::json>
{
private:
    template<class T>
    static constexpr bool JsonReadable
        = requires(const nlohmann::json& j) { j.template get<std::remove_cvref_t<T>>(); };

    template<class T>
    static constexpr bool JsonWritable
        = requires(nlohmann::json& j, const std::remove_cvref_t<T>& v) { j = v; };

public:
    RJsonResourceStream() = default;
    ~RJsonResourceStream() override = default;

    template<class T>
        requires(JsonReadable<T>)
    void read(std::string_view field, T& out, int flag = RFlag_Default) const
    {
        if (!_data.contains(field))
        {
            _logs.emplace_back(field.data(), RStatus::NotFound);
            if (flag & RFlag_Required)
            {
                throw std::runtime_error("Required field not found: '" + std::string(field) + "'");
            }
            return;
        }
        out = _data.at(field).get<T>();
    }

    template<class T, class T2>
        requires(JsonReadable<T>)
    void read(std::string_view field, T& out, T2&& defaultValue) const
    {
        if (!_data.contains(field))
        {
            _logs.emplace_back(field.data(), RStatus::NotFound);
            out = defaultValue;
        }
        else
        {
            read(field, out);
        }
    }

    template<class T>
        requires(JsonWritable<T>)
    void write(std::string_view field, const T& value)
    {
        _data[field] = value;
    }

    template<class T>
        requires(JsonWritable<T>)
    void write(const T& value)
    {
        for (auto&& [key, v] : value.items())
        {
            _data[key] = v;
        }
    }
};

//
//  ┏┳┓┏━┓┏━┓╻┏ ┏━┓
//  ┃┃┃┣━┫┣┳┛┣┻┓┗━┓
//  ╹ ╹╹ ╹╹┗╸╹ ╹┗━┛
//

/**
 * Use this macro for registration of your T.
 * The first parameter is T name, other params are serializable dependencies(i.g. a class's
 * parents). So, if you want to serialize the full parent-child chain of your classes, you MUST put
 * the parent of the current class. After that your current class, its parent, (its parent too...)
 * will be able to be serializable.
 */
#define R_FRIEND(Class, ...)                                                                       \
    friend struct R<Class>;                                                                        \
    static_assert(true, "")

/**
 * Put it before the enum-class definition to highlight it for JRM.
 * @code
 * ENUM_CLASS();
 * enum class Color {
 *     Red,
 *     Green,
 *     Blue
 * }
 * @endcode
 */
#define ENUM_CLASS(...) static_assert(true, "")

/**
 * Put it before the class definition to highlight it for JRM.
 * @code
 * CLASS();
 * class Foo{
 *     ...
 * };
 * @endcode
 */
#define CLASS(...) static_assert(true, "")

/**
 * Put it before the class's field definition to highlight it for JRM.
 * Also, it can't work without identifying your clas in the JRM system with 'CLASS()' keyword.
 * @code
 * CLASS(); // <--- you MUST put it
 * class Foo{
 *     FIELD();
 *     int a = 1;
 * };
 * @endcode
 *
 * The default behavior of 'FIELD' can be changed with enum class RFieldGen. Just put the necessary
 * extra parameters using comma separator to change the current behavior.
 * @code
 * CLASS();
 * class Foo{
 *     FIELD(R::NoDefaultValue, R::xyz);
 *     int a = 1;
 * };
 * @endcode
 */
#define FIELD(...) static_assert(true, "")
