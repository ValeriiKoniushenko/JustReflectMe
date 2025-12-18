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

#include "FileNavigationHelper.h"

#include <cstring>

namespace FileNavigator
{

    const char* GoToNextLine(const char* p)
    {
        if (auto* out = strchr(p, '\n'))
        {
            return *out == '\r' ? out + 1 : out;
        }
        return nullptr;
    }

    const char* FindOnThisLine(const char* source, const char* keyword)
    {
        const auto* endLine = strchr(source, '\n');
        if (!endLine)
        {
            return nullptr;
        }

        const auto* out = strstr(source, keyword);
        return out && out < endLine ? out : nullptr;
    }

    const char* GoToNotSpace(const char* source)
    {
        while (source && *source != '\0' && (*source == ' ' || *source == '\t'))
        {
            ++source;
        }

        return source;
    }

    std::string ReadAsIdentifier(const char* source)
    {
        if ((source = GoToNotSpace(source)))
        {
            if (!((*source > 'a' && *source <= 'z') || (*source > 'A' && *source <= 'Z')
                  || *source == '_'))
            {
                return {};
            }

            int count = 0;
            while (isalnum(source[count]) || source[count] == '_')
            {
                ++count;
            }

            return std::string(source, count);
        }

        return {};
    }

    std::size_t GetLineNumber(const char* source, std::size_t i)
    {
        if (!source) [[unlikely]]
        {
            return 0;
        }

        std::size_t count = 0;

        std::size_t iter = 0;
        while (source[iter] != '\0' && iter < i)
        {
            if (source[iter] == '\n')
            {
                ++count;
            }

            ++iter;
        }

        return count;
    }

    std::pair<std::size_t, std::size_t> GetLineNumberAndColumn(const char* source, std::size_t i)
    {
        if (!source) [[unlikely]]
        {
            return { 0, 0 };
        }

        std::size_t count = 0;

        std::size_t iter = 0;
        while (source[iter] != '\0' && iter < i)
        {
            if (source[iter] == '\n')
            {
                ++count;
            }

            ++iter;
        }

        return { count, i - iter };
    }

} // namespace FileNavigator
