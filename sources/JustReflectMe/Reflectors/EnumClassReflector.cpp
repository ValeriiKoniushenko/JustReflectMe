

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
#include "JustReflectMe/StringHelper.h"

#include <cstring>
#include <stdexcept>

using namespace FileNavigator;
using namespace StringHelper;

namespace JRM
{

    std::string EnumClassReflector::onGenerateHeaderFilePreNamespace() const
    {
        std::string result;
        result.reserve(512);

        result += "#include <optional>\n";
        result += "#include <string>\n";
        result += "#include <array>\n";
        result += "#include <unordered_map>\n";
        result += "\n";

        for (const auto& [_, data] : _data)
        {
            result += "enum class " + data.name;
            result += ";\n";
        }
        result += "\n";

        return result;
    }

    std::string EnumClassReflector::onGenerateHeaderFile() const
    {
        constexpr static const std::string nameMark = "@@NAME";
        constexpr static const std::string countMark = "@@COUNT";

        std::string result;
        result.reserve(1024);

        for (const auto& [_, data] : _data)
        {
            if (data.name.empty()) [[unlikely]]
            {
                throw GenerationException(
                    "Can't process generation of the header file due to unexpected empty the "
                    "enum's class name.");
            }

            std::string declarations = R"(
    namespace @@NAME
    {
        [[nodiscard]] constexpr const std::string& Name() { static constexpr std::string name = "@@NAME"; return name; }
        [[nodiscard]] constexpr std::size_t Size() noexcept { return @@COUNT; }

        [[nodiscard]] const std::string& ToString(::@@NAME value);
        [[nodiscard]] std::optional<::@@NAME> FromString(const std::string& value);

        [[nodiscard]] const std::array<::@@NAME, @@COUNT>& ToArrayC();
        [[nodiscard]] const std::array<std::string, @@COUNT>& ToArrayN();
        [[nodiscard]] const std::unordered_map<::@@NAME, std::string>& ToMapCN();
        [[nodiscard]] const std::unordered_map<std::string, ::@@NAME>& ToMapNC();
    } // namespace @@NAME
)";

            FindAndReplaceAll(declarations, nameMark, data.name);
            FindAndReplaceAll(declarations, countMark, std::to_string(data.constants.size()));

            result += declarations;
        }

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
            static const auto enumClassLength = strlen("enum class");
            p += enumClassLength;

            TokenData data;
            data.name = ReadAsIdentifier(p);
            if (data.name.empty())
            {
                throw SyntaxException("Not found enum's class identifier.", p - content.c_str());
            }

            prevP = p;
            p = FindFirstWithLineLimit(p, "{", 1);
            if (!p)
            {
                throw SyntaxException("Not found '{' after enum class identifier.",
                                      prevP - content.c_str());
            }

            const char* scopeStart = p;
            const char* scopeEnd = FindScopeEnd(p);
            if (!scopeEnd)
            {
                throw SyntaxException("Not found end of scope '}' for enum class '" + data.name
                                          + "'",
                                      scopeStart - content.c_str());
            }

            ++p;

            while (p < scopeEnd)
            {
                std::pair<std::string, std::string> nameAndValue;

                p = SkipAllBlanks(p);
                nameAndValue.first = ReadAsIdentifier(p);
                if (nameAndValue.first.empty())
                {
                    throw SyntaxException("Not found enum's constant identifier.",
                                          p - content.c_str());
                }
                p += nameAndValue.first.size();
                p = SkipAllBlanks(p);

                if (*p == '=')
                {
                    p = SkipAllBlanks(p + 1);
                    nameAndValue.second.assign(p, strchr(p, '\n') - p);

                    while (!nameAndValue.second.empty()
                           && (isspace(nameAndValue.second.back())
                               || nameAndValue.second.back() == ','))
                    {
                        nameAndValue.second.pop_back();
                    }
                }
                else
                {
                    if (data.constants.empty())
                    {
                        nameAndValue.second = "0";
                    }
                    else
                    {
                        nameAndValue.second = data.constants.back().first;
                        nameAndValue.second += "+1";
                    }
                }

                data.constants.emplace_back(std::move(nameAndValue));
                p = GoToNextLine(p);
            }

            _data.emplace(token, std::move(data));
        }
    }

} // namespace JRM