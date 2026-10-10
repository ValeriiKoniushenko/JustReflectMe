// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "StringHelper.h"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace StringHelper
{

    void FindAndReplaceAll(std::string& str, std::string_view from, const std::string& to)
    {
        if (from.empty())
        {
            return;
        }

        auto foundPos = str.find(from);
        while (foundPos != std::string::npos)
        {
            str.replace(foundPos, from.size(), to);
            foundPos = str.find(from, foundPos + to.size());
        }
    }

    void TrimInPlace(std::string& s)
    {
        const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::ranges::find_if(s, notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    }

    std::vector<std::string> SplitString(const std::string& str, char delim, bool isTrim)
    {
        std::vector<std::string> tokens;
        std::istringstream stream(str);
        std::string token;

        while (std::getline(stream, token, delim))
        {
            if (isTrim)
            {
                TrimInPlace(token);
            }

            tokens.push_back(std::move(token));
        }

        return tokens;
    }

    bool IsNumericTypeName(std::string_view n)
    {
        // Accept "std::uint8_t" and "::uint8_t" the same way as "uint8_t"
        if (n.starts_with("std::"))
        {
            n.remove_prefix(5);
        }
        else if (n.starts_with("::"))
        {
            n.remove_prefix(2);
        }

        static const std::unordered_set<std::string_view> numericTypes = {
            // floating point
            "float",
            "double",
            "long double",
            "_Float16",
            "float16_t",
            "float32_t",
            "float64_t",
            "float128_t",
            "bfloat16_t",

            // short
            "short",
            "short int",
            "signed short",
            "signed short int",
            "unsigned short",
            "unsigned short int",

            // int
            "int",
            "signed",
            "signed int",
            "unsigned",
            "unsigned int",

            // long
            "long",
            "long int",
            "signed long",
            "signed long int",
            "unsigned long",
            "unsigned long int",

            // long long
            "long long",
            "long long int",
            "signed long long",
            "signed long long int",
            "unsigned long long",
            "unsigned long long int",

            // 128-bit extensions (GCC/Clang)
            "__int128",
            "signed __int128",
            "unsigned __int128",
            "__int128_t",
            "__uint128_t",

            // fixed-width
            "int8_t",
            "int16_t",
            "int32_t",
            "int64_t",
            "uint8_t",
            "uint16_t",
            "uint32_t",
            "uint64_t",
            "int_fast8_t",
            "int_fast16_t",
            "int_fast32_t",
            "int_fast64_t",
            "uint_fast8_t",
            "uint_fast16_t",
            "uint_fast32_t",
            "uint_fast64_t",
            "int_least8_t",
            "int_least16_t",
            "int_least32_t",
            "int_least64_t",
            "uint_least8_t",
            "uint_least16_t",
            "uint_least32_t",
            "uint_least64_t",
            "intmax_t",
            "uintmax_t",
            "intptr_t",
            "uintptr_t",

            // size-like
            "size_t",
            "ssize_t",
            "ptrdiff_t",

            // character types (integers, remove if you don't want them)
            "char",
            "signed char",
            "unsigned char",
            "wchar_t",
            "char8_t",
            "char16_t",
            "char32_t",
        };

        return numericTypes.contains(n);
    }

} // namespace StringHelper
