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

#include <string_view>

#include "NlohmannJson.h"

class RBaseResourceStreamImpl
{
public:
    RBaseResourceStreamImpl() = default;
    virtual ~RBaseResourceStreamImpl() = default;
};

template<class T>
concept IsResourceStreamImpl = requires(T t, int some_variable) {
    { std::derived_from<T, RBaseResourceStreamImpl> };
    { t.template read<int>("some_field_name", some_variable) } -> std::same_as<void>;
    { t.template write<int>("some_field_name", 12345) } -> std::same_as<void>;
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
