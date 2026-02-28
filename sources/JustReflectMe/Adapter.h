/*
 * MIT License
 *
 * Copyright (c) 2018-2026 Valerii Koniushenko
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

#include <string_view>

template<class DataT>
class RBaseResourceStreamImpl
{
public:
    RBaseResourceStreamImpl() = default;
    virtual ~RBaseResourceStreamImpl() = default;

    [[nodiscard]] DataT& data() noexcept { return _data; }
    [[nodiscard]] const DataT& data() const noexcept { return _data; }

protected:
    DataT _data;
};

template<class T>
concept DerivedFromAnyRBaseResourceStreamImpl
    = requires(std::remove_cvref_t<T>* p) { []<class U>(RBaseResourceStreamImpl<U>*) {}(p); };

template<class T>
concept IsResourceStreamImpl = requires(T t, int some_variable) {
    requires DerivedFromAnyRBaseResourceStreamImpl<T>;
    { t.template read<int>("some_field_name", some_variable) } -> std::same_as<void>;
    { t.template write<int>("some_field_name", 12345) } -> std::same_as<void>;
};

class RJsonResourceStream : public RBaseResourceStreamImpl<nlohmann::json>
{
public:
    RJsonResourceStream() = default;
    ~RJsonResourceStream() override = default;

    template<class T>
    void read(std::string_view field, T& out)
    {
        out = _data.at(field).get<T>();
    }
    template<class T>
    void write(std::string_view field, const T& value)
    {
        _data[field] = value;
    }
};

template<IsResourceStreamImpl RImpl>
struct RResourceStream
{
public:
    RResourceStream() = default;
    virtual ~RResourceStream() = default;

    template<class T>
    void write(std::string_view fieldName, T& value)
    {
        impl.template write<T>(fieldName, value);
    }

    template<class T>
    [[nodiscard]] T read(std::string_view fieldName)
    {
        return impl.template read<T>(fieldName);
    }

    [[nodiscard]] const auto& getData() const noexcept { return impl.data(); }
    [[nodiscard]] auto& getData() noexcept { return impl.data(); }

protected:
    RImpl impl;
};

struct RClassField
{
    constexpr RClassField(std::string_view type, std::string_view name) noexcept
        : type(type),
          name(name)
    {
    }

    std::string_view type;
    std::string_view name;
};

template<class T>
struct R
{
};

#define R_FRIEND(Class) friend struct R<Class>

#define ENUM_CLASS(...) static_assert(true, "")

#define CLASS(...) static_assert(true, "")
#define FIELD(...) static_assert(true, "")
