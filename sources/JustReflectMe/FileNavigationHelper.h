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

#include "FileProcessor.h"

#include <string>
#include <vector>

namespace FileNavigator
{

    [[nodiscard]] const char* GoToLineStart(const char* p, const char* begin);
    [[nodiscard]] const char* GoToPrevLine(const char* p, const char* begin);
    [[nodiscard]] const char* GoToNextLine(const char* p);

    // limit = 0 -- only on this line
    [[nodiscard]] const char* FindFirstWithLineLimit(const char* source, const char* keyword,
                                                     std::size_t limit);
    [[nodiscard]] const char* FindOnThisLine(const char* source, const char* keyword);
    [[nodiscard]] const char* FindWordOnThisLine(std::string_view content, std::string_view word);

    [[nodiscard]] const char* GoToSpace(const char* source);
    [[nodiscard]] const char* GoToBlank(const char* source);
    [[nodiscard]] const char* GoToNotSpace(const char* source);
    [[nodiscard]] const char* SkipAllBlanks(const char* source);
    [[nodiscard]] std::string ReadAsIdentifier(const char* source);
    [[nodiscard]] std::size_t GetLineNumber(const char* source, std::size_t i);
    [[nodiscard]] std::pair<std::size_t, std::size_t> GetLineNumberAndColumn(const char* source,
                                                                             std::size_t i);
    [[nodiscard]] const char* FindScopeEnd(const char* source);
    [[nodiscard]] bool IsWord(std::string_view content, std::string_view word, std::size_t wordPos);
    [[nodiscard]] int StartWith(const char* content, const std::vector<std::string_view>& prefixes);
    [[nodiscard]] bool StartWith(const char* content, std::string_view prefix);

    [[nodiscard]] inline bool IsNewLine(int ch) noexcept
    {
        return ch == '\n' || ch == JRM::PostProcessedFile::newLinePlaceholder;
    }

    [[nodiscard]] inline bool IsSpace(int ch) noexcept
    {
        return std::isspace(ch) || IsNewLine(ch);
    }

    struct Typename
    {
        enum class Attribute
        {
            None,
            Constexpr,
            Static,
            Inline
        };

        void setAttributeFromStr(std::string_view str);
        [[nodiscard]] std::string getNameWithCV() const;

        std::string name;
        Attribute attribute = Attribute::None;
        bool isConst = false;
    };
    [[nodiscard]] Typename ReadAsTypename(const char* source, int& offset);

} // namespace FileNavigator
