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

#include "nlohmann/json.hpp"

#include <string>
#include <string_view>
#include <vector>

// ==================================================
//                     MISC
// ==================================================

/**
 * @brief Metadata for one reflected class field.
 */
struct RClassField
{
    std::string_view type;
    std::string_view name;
    std::vector<std::string_view> attribs;
};

/**
 * @brief Reflection flags and customization point for a reflected type.
 * @tparam T Reflected type associated with the generated `R<T>` specialization.
 */
template<class T = void>
struct R
{
    /** @brief Marks a field attribute supplied to the reflection metadata. */
    constexpr static uint64_t Attr = 1 << 1;

    /** @brief Prevents the generated deserializer from assigning a field's C++ default value. */
    constexpr static uint64_t NoDefaultValue = 1 << 2;

    /** @brief Marks a field as required; a missing value is logged and causes deserialization to
     * throw. */
    constexpr static uint64_t Required = 1 << 3;

    /** @brief Treats a missing serialized field as non-fatal. */
    constexpr static uint64_t NonRequired = 1 << 4;

    /** @brief Default missing-field behavior, equivalent to `NonRequired`. */
    constexpr static uint64_t Default = R::NonRequired;
};

using RPoint = R<void>;

/**
 * @brief Status recorded while reading serialized fields.
 */
enum class RStatus
{
    Ok,
    NotFound
};

/**
 * @brief Converts a serialization status to text.
 * @param status Status to convert.
 * @return `"Ok"`, `"NotFound"`, or `"Unknown"`.
 */
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

// ==================================================
//              RBaseResourceStreamImpl
// ==================================================
/**
 * @brief Stores format-specific data and serialization diagnostics.
 * @tparam DataT Underlying representation used by the resource stream.
 */
template<class DataT>
class RBaseResourceStreamImpl
{
public:
    using DataType = DataT;

    RBaseResourceStreamImpl() = default;
    RBaseResourceStreamImpl(const RBaseResourceStreamImpl&) = default;
    RBaseResourceStreamImpl& operator=(const RBaseResourceStreamImpl&) = default;
    RBaseResourceStreamImpl(RBaseResourceStreamImpl&&) noexcept = default;
    RBaseResourceStreamImpl& operator=(RBaseResourceStreamImpl&&) noexcept = default;
    virtual ~RBaseResourceStreamImpl() = default;

    /**
     * @brief Returns the mutable underlying representation.
     */
    [[nodiscard]] DataT& data() noexcept { return _data; }

    /**
     * @brief Returns the read-only underlying representation.
     */
    [[nodiscard]] const DataT& data() const noexcept { return _data; }

    /**
     * @brief Returns diagnostics collected while reading fields.
     */
    [[nodiscard]] const RLogsCollector& logs() const noexcept { return _logs; }

    /**
     * @brief Returns the mutable diagnostic collection.
     */
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
    { t.template read<int>("foo", v, RPoint::Default) } -> std::same_as<void>;
    { t.template read<int>("foo", v, 100) } -> std::same_as<void>;
    { t.template write<int>("foo", 12345) } -> std::same_as<void>;
};

// ==============================================
//              RResourceStream
// ==============================================
/**
 * @brief Format-independent facade for reading and writing reflected values.
 * @tparam RImpl Resource-stream implementation that supplies the underlying data format.
 */
template<IsResourceStreamImpl RImpl>
struct RResourceStream
{
public:
    RResourceStream() = default;
    RResourceStream(const RResourceStream&) = default;
    RResourceStream& operator=(const RResourceStream&) = default;
    RResourceStream(RResourceStream&&) noexcept = default;
    RResourceStream& operator=(RResourceStream&&) noexcept = default;
    virtual ~RResourceStream() = default;

    /**
     * @brief Initializes a stream with an existing serialized representation.
     * @param out Data to copy into the stream implementation.
     */
    RResourceStream(const typename RImpl::DataType& out) { impl.data() = out; }

    /**
     * @brief Writes a value under a named field.
     * @param fieldName Serialized field name.
     * @param value Value to serialize.
     */
    template<class T>
    void write(std::string_view fieldName, const T& value)
    {
        impl.template write<T>(fieldName, value);
    }

    /**
     * @brief Writes a value using the implementation's whole-object operation.
     * @param value Value to serialize.
     */
    template<class T>
    void write(const T& value)
    {
        impl.template write<T>(value);
    }

    /**
     * @brief Reads a required field from the serialized representation.
     * @param fieldName Serialized field name.
     * @param value Destination for the deserialized value.
     */
    template<class T>
    void read(std::string_view fieldName, T& value) const
    {
        impl.template read<T>(fieldName, value);
    }

    /**
     * @brief Reads a field and supplies a fallback when it is absent.
     * @param fieldName Serialized field name.
     * @param value Destination for the deserialized value.
     * @param defaultValue Value assigned when the field is absent.
     * @param flag Missing-field behavior flags from `R`.
     */
    template<class T, class T2>
    void read(std::string_view fieldName, T& value, T2&& defaultValue,
              int flag = RPoint::Default) const
    {
        impl.template read<T>(fieldName, value, std::forward<decltype(defaultValue)>(defaultValue),
                              flag);
    }

    /**
     * @brief Returns the serialized representation.
     */
    [[nodiscard]] const auto& getData() const noexcept { return impl.data(); }
    /**
     * @brief Returns the mutable serialized representation.
     */
    [[nodiscard]] auto& getData() noexcept { return impl.data(); }

    /**
     * @brief Returns diagnostics collected during deserialization.
     */
    [[nodiscard]] const RLogsCollector& logs() const noexcept { return impl.logs(); }

    /**
     * @brief Returns the mutable deserialization diagnostics.
     */
    [[nodiscard]] RLogsCollector& logs() noexcept { return impl.logs(); }

protected:
    RImpl impl;
};

// ================================================
//              RJsonResourceStream
// ================================================
/**
 * @brief JSON implementation of the reflection resource stream.
 *
 * Missing fields receive the supplied default value and are recorded in `logs()`. A read marked
 * with `RPoint::Required` additionally throws when its field is absent.
 * The set of types that can be serialized and deserialized follows nlohmann::json's conversion
 * support. It can be extended for user-defined types by providing global `to_json(BasicJsonType&,
 * const T&)` and `from_json(const BasicJsonType&, T&)` functions visible to nlohmann::json.
 */
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
    RJsonResourceStream(const RJsonResourceStream&) = default;
    RJsonResourceStream& operator=(const RJsonResourceStream&) = default;
    RJsonResourceStream(RJsonResourceStream&&) noexcept = default;
    RJsonResourceStream& operator=(RJsonResourceStream&&) noexcept = default;
    ~RJsonResourceStream() override = default;

    /**
     * @brief Reads a JSON field or assigns a default value when it is absent.
     * @param field JSON object key.
     * @param out Destination for the decoded value.
     * @param defaultValue Fallback value for a missing key.
     * @param flag Missing-field behavior flags from `R`.
     * @throw std::runtime_error When the field is absent and `RPoint::Required` is set.
     */
    template<class T, class T2>
        requires(JsonReadable<T>)
    void read(std::string_view field, T& out, T2&& defaultValue, int flag = RPoint::Default) const
    {
        using namespace std::string_literals;

        if (!_data.contains(field))
        {
            _logs.emplace_back(field.data(), RStatus::NotFound);
            out = std::forward<T2>(defaultValue);
            if (flag & RPoint::Required)
            {
                throw std::runtime_error("Required field not found: '"s + field.data() + "'");
            }
        }
        else
        {
            out = _data.at(field).get<T>();
        }
    }

    /**
     * @brief Writes a value to a named JSON field.
     * @param field JSON object key.
     * @param value Value to encode.
     */
    template<class T>
        requires(JsonWritable<T>)
    void write(std::string_view field, const T& value)
    {
        _data[field] = value;
    }

    /**
     * @brief Merges the fields of a JSON-compatible object into the stream.
     * @param value Object whose fields should be copied into the JSON representation.
     */
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

// ==================================================
//                 CODE MARKS
// ==================================================

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
 * The default behavior of 'FIELD' can be changed with enum class RPoint. Just put the necessary
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

namespace RInternal
{

    template<IsResourceStreamImpl RImpl = RJsonResourceStream, class T, class... Args>
    [[nodiscard]] T Deserialize(const RResourceStream<RImpl>& s, bool noSignals = false,
                                Args&&... args)
    {
        T out(std::forward<Args>(args)...);
        Deserialize<RImpl>(s, out, noSignals);
        return out;
    }

    [[nodiscard]] std::unordered_map<std::string, RClassField> GetClassFieldsAsMap(
        const std::vector<RClassField>& fields);

} // namespace RInternal
