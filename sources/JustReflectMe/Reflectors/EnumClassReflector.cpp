

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

#include "EnumClassReflector.h"

#include "JustReflectMe/FileNavigationHelper.h"

#include <cstring>
#include <stdexcept>

using namespace FileNavigator;

namespace JRM
{

    std::string EnumClassReflector::onGenerateHeaderFile() const
    {
        std::string result;
        result.reserve(1024);

        return result;
    }

    std::string EnumClassReflector::onGenerateSourceFile() const
    {
        std::string result;
        result.reserve(1024);
        return result;
    }

    void EnumClassReflector::onScan(const std::string& content)
    {
        for (const auto& token : _tokens)
        {
            if (!token.isValid()) [[unlikely]]
            {
                throw std::runtime_error("Invalid token was found.");
            }

            if (token.begin >= content.size()) [[unlikely]]
            {
                throw std::runtime_error(
                    "Token begin position is out of range: " + std::to_string(token.begin)
                    + " But content length is: " + std::to_string(content.size()));
            }

            const char* p = content.c_str() + token.begin;
            const char* prevP = p;

            // Validating define
            static const auto keywordLength = strlen(getTriggerKeyword());
            if (strncmp(p, getTriggerKeyword(), keywordLength) != 0) [[unlikely]]
            {
                throw SyntaxException("Invalid keyword was found. But expected: "
                                          + std::string(getTriggerKeyword()),
                                      prevP - content.c_str());
            }

            prevP = p;
            p = GoToNextLine(p);
            if (!p)
            {
                throw SyntaxException(
                    std::string(getTriggerKeyword())
                        + " keyword found, but 'enum class' wasn't found after it.",
                    prevP - content.c_str());
            }

            prevP = p;
            p = FindOnThisLine(p, "enum class");
            if (!p)
            {
                throw SyntaxException(
                    std::string(getTriggerKeyword())
                        + " keyword found, but 'enum class' wasn't found after it.",
                    prevP - content.c_str());
            }

            TokenData data;
            data.name = ReadAsIdentifier(p);
            if (data.name.empty())
            {
                throw SyntaxException("Not found enum's class identifier.", p - content.c_str());
            }

            prevP = p;
            p = FindOnThisLine(p, "{");
            if (!p)
            {
                throw SyntaxException("Not found '{' after enum's class identifier.", prevP - content.c_str());
            }
        }
    }

} // namespace JRM