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

#include "FileNavigationHelper.h"

#include <cstring>

namespace FileNavigator
{

    const char* GoToLineStart(const char* p, const char* begin)
    {
        if (!begin || !p) [[unlikely]]
        {
            return nullptr;
        }

        while (p > begin && *p != '\n')
        {
            --p;
        }
        return ++p;
    }

    const char* GoToPrevLine(const char* p, const char* begin)
    {
        if (!begin || !p) [[unlikely]]
        {
            return nullptr;
        }

        p = GoToLineStart(p, begin);
        if (p - 1 > begin)
        {
            p = GoToLineStart(p - 1, begin);
        }
        return ++p;
    }

    const char* GoToNextLine(const char* p)
    {
        if (auto* out = strchr(p, '\n'))
        {
            return *out == '\r' ? out + 2 : out + 1;
        }
        return nullptr;
    }

    const char* FindFirstWithLineLimit(const char* source, const char* keyword, std::size_t limit)
    {
        if (!source || !keyword) [[unlikely]]
        {
            return nullptr;
        }

        const auto* result = strstr(source, keyword);
        if (!result)
        {
            return nullptr;
        }

        std::size_t count = 0;
        while (*source && *result && source < result)
        {
            if (*source == '\n')
            {
                if (++count > limit)
                {
                    return nullptr;
                }
            }

            ++source;
        }

        return result;
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

    const char* FindWordOnThisLine(const std::string& content, std::string_view word)
    {
        const char* result = FindOnThisLine(content.c_str(), word.data());
        return result && isWord(content, word, result - content.c_str()) ? result : nullptr;
    }

    const char* GoToSpace(const char* source)
    {
        while (source && *source != '\0' && !(*source == ' ' || *source == '\t'))
        {
            ++source;
        }

        return source;
    }

    const char* GoToNotSpace(const char* source)
    {
        while (source && *source != '\0' && (*source == ' ' || *source == '\t'))
        {
            ++source;
        }

        return source;
    }

    const char* SkipAllBlanks(const char* source)
    {
        while (isspace(source[0]))
        {
            ++source;
        }

        return source;
    }

    std::string ReadAsIdentifier(const char* source)
    {
        if ((source = GoToNotSpace(source)))
        {
            if (!isalpha(*source) && *source != '_')
            {
                return {};
            }

            int count = 0;
            while (isalnum(source[count]) || source[count] == '_' || source[count] == ':')
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
        while (iter < i && source[iter] != '\0')
        {
            if (source[iter] == '\n')
            {
                ++count;
            }

            ++iter;
        }

        return count + 1;
    }

    std::pair<std::size_t, std::size_t> GetLineNumberAndColumn(const char* source, std::size_t i)
    {
        if (!source) [[unlikely]]
        {
            return { 0, 0 };
        }

        std::size_t count = 0;

        std::size_t iter = 0;
        while (iter < i && source[iter] != '\0')
        {
            if (source[iter] == '\n')
            {
                ++count;
            }

            ++iter;
        }

        return { count + 1, i - iter + 1 };
    }

    const char* FindScopeEnd(const char* source)
    {
        if (!source) [[unlikely]]
        {
            return nullptr;
        }

        if (!(*source == '{' || *source == '(' || *source == '<' || *source == '[')) [[unlikely]]
        {
            return nullptr;
        }

        const char openSign = *source;
        char closeSign = 0;
        if (openSign == '{')
        {
            closeSign = '}';
        }
        else if (openSign == '(')
        {
            closeSign = ')';
        }
        else if (openSign == '<')
        {
            closeSign = '>';
        }
        else
        {
            closeSign = ']';
        }

        int count = 0;
        while (*source)
        {
            if (*source == openSign)
            {
                ++count;
            }
            else if (*source == closeSign)
            {
                if (--count == 0)
                {
                    return source;
                }
            }

            ++source;
        }

        return nullptr;
    }

    bool isWord(const std::string& content, std::string_view word, std::size_t wordPos)
    {
        if (wordPos > 0 && (std::isalnum(content[wordPos - 1]) || content[wordPos - 1] == '_'))
        {
            return false;
        }

        const auto end = word.size() + wordPos;
        if (end < content.size() && (std::isalnum(content[end]) || content[end] == '_'))
        {
            return false;
        }

        return true;
    }

} // namespace FileNavigator
