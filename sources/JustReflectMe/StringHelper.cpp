/*
 * MIT License
 *
 * Copyright (c) 2018-2025 Valerii Koniushenko
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

#include "StringHelper.h"

#include <algorithm>
#include <sstream>

namespace StringHelper
{

    void FindAndReplaceAll(std::string& str, std::string_view from, const std::string& to)
    {
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