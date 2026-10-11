// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "Number.h"

#include <cctype>
#include <charconv>
#include <optional>

namespace JRM
{

    std::optional<Number> TryParseNumber(const char* s)
    {
        if (!s)
        {
            return std::nullopt;
        }

        const char* first = s;
        const char* last = s;
        while (*last && (*last == '-' || *last == '.' || std::isdigit(*last)))
        {
            ++last;
        }

        // 1. Try integer first
        int64_t i{};
        if (auto [p, ec] = std::from_chars(first, last, i); ec == std::errc{} && p == last)
        {
            return i;
        }

        // 2. Otherwise try floating point
        double d{};
        if (auto [p, ec] = std::from_chars(first, last, d); ec == std::errc{} && p == last)
        {
            return d;
        }

        return std::nullopt; // not a number
    }

} // namespace JRM
