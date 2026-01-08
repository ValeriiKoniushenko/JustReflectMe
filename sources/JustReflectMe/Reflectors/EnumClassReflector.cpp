

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

#include "EnumClassReflector.h"

#include "JustReflectMe/FileData.h"
#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/StringHelper.h"

#include <cstring>
#include <stdexcept>

using namespace FileNavigator;
using namespace StringHelper;

namespace JRM
{

    std::string EnumClassReflector::TokenData::fullNamePath() const
    {
        if (parentSpace.empty())
        {
            return name;
        }

        return parentSpace + "::" + name;
    }

    std::string EnumClassReflector::onGenerateHeaderFilePreNamespace(FileData& data) const
    {
        std::string result;
        result.reserve(512);

        result += "#include <optional>\n";
        result += "#include <string>\n";
        result += "#include <array>\n";
        result += "#include <unordered_map>\n";
        result += "\n";

        // for (const auto& [_, data] : _data)
        // {
        //     result += "enum class " + data.name;
        //     result += ";\n";
        // }

        result += "\n";

        return result;
    }

    std::string EnumClassReflector::onGenerateHeaderFile(FileData& fileData) const
    {
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

            std::string finalString;
            finalString.reserve(1024 * 4);

            finalString += generateDeclaration(data);
            finalString += generateImplementation(data, true);

            result += finalString;
        }

        return result;
    }

    std::string EnumClassReflector::onGenerateSourceFile(FileData& data) const
    {
        return {};
    }

    void EnumClassReflector::onScan(const FileData& fileData)
    {
        const auto& content = fileData.getContent();

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

            if (const Scope* scope = fileData.getScopes().getScopeAt(p))
            {
                data.parentSpace = PrettyPrintScope(scope);
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
                throw SyntaxException("Not found end of scope '}' for enum class '"
                                          + data.fullNamePath() + "'",
                                      scopeStart - content.c_str());
            }

            ++p;

            while (p && p < scopeEnd)
            {
                std::pair<std::string, std::string> nameAndValue;

                p = SkipAllBlanks(p);
                nameAndValue.first = ReadAsIdentifier(p);
                if (nameAndValue.first.empty())
                {
                    break; // no constants
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

    std::string EnumClassReflector::generateDeclaration(const TokenData& data) const
    {
        std::string finalString = R"(
    namespace @@NAME_
    {

        // =================== DECLARATIONS =====================
        [[nodiscard]] const std::string& Name();
        [[nodiscard]] const std::string& ParentScope();
        [[nodiscard]] constexpr std::size_t Size() noexcept { return @@COUNT_; }

        [[nodiscard]] const std::string& ToString(::@@NAME_ value);
        [[nodiscard]] std::optional<::@@NAME_> FromString(const std::string& value);

        [[nodiscard]] const std::array<::@@NAME_, @@COUNT_>& ToArrayC();
        [[nodiscard]] const std::array<std::string, @@COUNT_>& ToArrayN();
        [[nodiscard]] const std::unordered_map<::@@NAME_, std::string>& ToMapCN();
        [[nodiscard]] const std::unordered_map<std::string, ::@@NAME_>& ToMapNC();

    } // namespace @@NAME_
)";

        FindAndReplaceAll(finalString, nameMark, data.fullNamePath());
        FindAndReplaceAll(finalString, realNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, countMark, std::to_string(data.constants.size()));

        return finalString;
    }

    std::string EnumClassReflector::generateImplementation(const TokenData& data, bool inlined) const
    {
        std::string finalString = R"(
    namespace @@NAME_
    {

        // =================== IMPLEMENTATIONS =====================
        @@FUNC_PREF_ const std::string& Name() { static const std::string name = "@@REAL_NAME_"; return name; }
        @@FUNC_PREF_ const std::string& ParentScope() { static const std::string name = "@@PARENTS_"; return name; }

        @@FUNC_PREF_ const std::string& ToString(::@@NAME_ value)
        {
            const auto& data = Reflect::@@NAME_::ToMapCN();
            const auto it = data.find(value);
            if (it != data.end()) [[likely]]
            {
                return it->second;
            }
            static constexpr std::string empty{};
            return empty;
        }

        @@FUNC_PREF_ std::optional<::@@NAME_> FromString(const std::string& value)
        {
            const auto& data = Reflect::@@NAME_::ToMapNC();
            const auto it = data.find(value);
            if (it != data.end()) [[likely]]
            {
                return it->second;
            }
            return std::nullopt;
        }

        @@FUNC_PREF_ const std::array<::@@NAME_, @@COUNT_>& ToArrayC()
        {
            static constexpr std::array<::@@NAME_, @@COUNT_> constants = {
@@TO_ARRAY_C_
            };

            return constants;
        }

        @@FUNC_PREF_ const std::array<std::string, @@COUNT_>& ToArrayN()
        {
            static constexpr std::array<std::string, @@COUNT_> names = {
@@TO_ARRAY_N_
            };

            return names;
        }

        @@FUNC_PREF_ const std::unordered_map<::@@NAME_, std::string>& ToMapCN()
        {
            static const std::unordered_map<::@@NAME_, std::string> map = {
@@TO_ARRAY_CN_
            };

            return map;
        }

        @@FUNC_PREF_ const std::unordered_map<std::string, ::@@NAME_>& ToMapNC()
        {
            static const std::unordered_map<std::string, ::@@NAME_> map = {
@@TO_ARRAY_NC_
            };

            return map;
        }

    } // namespace @@NAME_
)";

        // Impl ToArrayC
        {
            std::string str;
            str.reserve(32 * (data.constants.size() + 1));
            for (const auto& [name, _] : data.constants)
            {
                str += "\t\t\t\t::";
                str += data.fullNamePath();
                str += "::";
                str += name;
                str += ",\n";
            }

            if (!str.empty())
            {
                if (str.back() == '\n') [[likely]]
                {
                    str.pop_back();
                }
                if (str.back() == ',') [[likely]]
                {
                    str.pop_back();
                }
            }

            FindAndReplaceAll(finalString, "@@TO_ARRAY_C_", str);
        }

        // Impl ToArrayN
        {
            std::string str;
            str.reserve(48 * (data.constants.size() + 1));
            for (const auto& [name, _] : data.constants)
            {
                str += "\t\t\t\tstd::string(\"";
                str += name;
                str += "\"),\n";
            }

            if (!str.empty())
            {
                if (str.back() == '\n') [[likely]]
                {
                    str.pop_back();
                }
                if (str.back() == ',') [[likely]]
                {
                    str.pop_back();
                }
            }

            FindAndReplaceAll(finalString, "@@TO_ARRAY_N_", str);
        }

        // Impl ToArrayCN
        {
            std::string str;
            str.reserve(64 * (data.constants.size() + 1));
            for (const auto& [name, _] : data.constants)
            {
                str += "\t\t\t\t{ ::";
                str += data.fullNamePath();
                str += "::";
                str += name;
                str += ", \"";
                str += name;
                str += "\" },\n";
            }

            if (!str.empty())
            {
                if (str.back() == '\n') [[likely]]
                {
                    str.pop_back();
                }
                if (str.back() == ',') [[likely]]
                {
                    str.pop_back();
                }
            }

            FindAndReplaceAll(finalString, "@@TO_ARRAY_CN_", str);
        }

        // Impl ToArrayNC
        {
            std::string str;
            str.reserve(64 * (data.constants.size() + 1));
            for (const auto& [name, _] : data.constants)
            {
                str += "\t\t\t\t{ \"";
                str += name;
                str += "\", ::";
                str += data.fullNamePath();
                str += "::";
                str += name;
                str += " },\n";
            }

            if (!str.empty())
            {
                if (str.back() == '\n') [[likely]]
                {
                    str.pop_back();
                }
                if (str.back() == ',') [[likely]]
                {
                    str.pop_back();
                }
            }

            FindAndReplaceAll(finalString, "@@TO_ARRAY_NC_", str);
        }

        FindAndReplaceAll(finalString, nameMark, data.fullNamePath());
        FindAndReplaceAll(finalString, realNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, countMark, std::to_string(data.constants.size()));

        std::string funcPref = inlined ? "inline" : "";
        FindAndReplaceAll(finalString, funcPrefMark, funcPref);

        return finalString;
    }

} // namespace JRM