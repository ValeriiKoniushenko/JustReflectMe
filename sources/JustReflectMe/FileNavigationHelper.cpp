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

        while (p > begin && !IsNewLine(*(p - 1)))
        {
            --p;
        }
        return p;
    }

    const char* GoToPrevLine(const char* p, const char* begin)
    {
        if (!begin || !p) [[unlikely]]
        {
            return nullptr;
        }

        p = GoToLineStart(p, begin);
        if (p <= begin)
        {
            return begin;
        }

        return GoToLineStart(p - 1, begin);
    }

    const char* GoToNextLine(const char* p)
    {
        if (auto* out = strpbrk(p, "\n\x1D")) // \x1D is newLinePlaceholder
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
            if (IsNewLine(*source))
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
        const auto* endLine = strpbrk(source, "\n\x1D"); // \x1D is newLinePlaceholder
        const auto* out = strstr(source, keyword);
        return out && (!endLine || out < endLine) ? out : nullptr;
    }

    const char* FindWordOnThisLine(std::string_view content, std::string_view word)
    {
        const char* result = FindOnThisLine(content.data(), word.data());
        return result && IsWord(content.data(), word, result - content.data()) ? result : nullptr;
    }

    const char* GoToSpace(const char* source)
    {
        while (source && *source != '\0' && !(*source == ' ' || *source == '\t'))
        {
            ++source;
        }

        return source;
    }

    const char* GoToBlank(const char* source)
    {
        while (source && *source != '\0' && !IsSpace(*source))
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
        while (IsSpace(source[0]))
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

    Typename ReadAsTypename(const char* source, int& offset)
    {
        if (!source || (!isalpha(*source) && *source != '_'))
        {
            return {};
        }
        Typename result;

        const char* const originalSource = source;

        constexpr std::string_view kConstexpr = "constexpr";
        constexpr std::string_view kConst = "const";
        constexpr std::string_view kStatic = "static";
        constexpr std::string_view kInline = "inline";
        static const std::vector<std::string_view> kKeywords = { kConstexpr, kStatic, kInline };

        if (const int i = StartWith(source, { kConstexpr, kStatic, kInline }); i != -1)
        {
            source = GoToNotSpace(GoToSpace(source));
            result.setAttributeFromStr(kKeywords[i]);
        }

        if (StartWith(source, kConst))
        {
            result.isConst = true;
            source = GoToNotSpace(GoToSpace(source));
        }

        int triangScopes = 0;
        while (*source)
        {
            result.name.push_back(*source);

            if (*source == '<')
            {
                ++triangScopes;
            }
            else if (*source == '>')
            {
                --triangScopes;
                ++source;
                continue;
            }

            if (triangScopes == 0 && *source != ':' && !std::isalnum(*source) && *source != '_')
            {
                result.name.pop_back();
                break;
            }

            ++source;
        }

        if (!result.name.empty() && result.name.back() == ',')
        {
            result.name.pop_back();
            --source;
        }

        const auto* const qualifier = GoToNotSpace(source);
        if (*qualifier && StartWith(qualifier, { kConst }))
        {
            result.isConst = true;
            source = qualifier;
        }

        // Try to determine '*' and '&' at the end of the type
        int transactionOffset = 0;
        bool foundTypeSuffix = false;
        while (source[transactionOffset] == ' ') // skip blanks between type and '*' or '&'
        {
            ++transactionOffset;
        }
        while (source[transactionOffset] == '*' || source[transactionOffset] == '&')
        {
            result.name.push_back(source[transactionOffset]);
            ++transactionOffset;
            foundTypeSuffix = true;
        }
        if (foundTypeSuffix)
        {
            source += transactionOffset;
        }

        offset = static_cast<int>(source - originalSource);
        return result;
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
            if (IsNewLine(source[iter]))
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
        std::size_t lineStart = 0;
        while (iter < i && source[iter] != '\0')
        {
            if (IsNewLine(source[iter]))
            {
                ++count;
                lineStart = iter + 1;
            }

            ++iter;
        }

        return { count + 1, i - lineStart + 1 };
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

    bool IsWord(std::string_view content, std::string_view word, std::size_t wordPos)
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

    int StartWith(const char* content, const std::vector<std::string_view>& prefixes)
    {
        for (std::size_t i = 0; i < prefixes.size(); ++i)
        {
            if (strncmp(content, prefixes[i].data(), prefixes[i].size()) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    bool StartWith(const char* content, std::string_view prefix)
    {
        return strncmp(content, prefix.data(), prefix.size()) == 0;
    }

    bool LeadToNewLine(const char* content) noexcept
    {
        while (content && std::isspace(*content))
        {
            if (*content == '\n')
            {
                return true;
            }

            ++content;
        }

        return false;
    }

    void Typename::setAttributeFromStr(std::string_view str)
    {
        if (str == "constexpr")
        {
            attribute = Attribute::Constexpr;
        }
        else if (str == "static")
        {
            attribute = Attribute::Static;
        }
        else if (str == "inline")
        {
            attribute = Attribute::Inline;
        }
    }

    std::string Typename::getNameWithCV() const
    {
        if (isConst)
        {
            return "const " + name;
        }
        return name;
    }

} // namespace FileNavigator
