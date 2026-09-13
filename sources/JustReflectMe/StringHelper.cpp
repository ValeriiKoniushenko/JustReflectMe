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

} // namespace StringHelper
