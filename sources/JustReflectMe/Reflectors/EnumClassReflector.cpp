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

#include "../Config.h"
#include "JustReflectMe/FileData.h"
#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/StringHelper.h"

#include <cstring>
#include <iostream>
#include <ranges>
#include <stdexcept>

using namespace FileNavigator;
using namespace StringHelper;
using namespace std::string_literals;

namespace JRM
{

    std::set<std::string> EnumClassReflector::getIncludes() const
    {
        auto out = BaseReflector::getIncludes();
        out.emplace("optional");
        out.emplace("unordered_map");
        out.emplace("array");
        return out;
    }

    std::string EnumClassReflector::onGenerateHeaderFile(FileData& fileData,
                                                         const Config& config) const
    {
        std::string result;
        result.reserve(1024 * 4);

        for (const auto& data : _data | std::views::values)
        {
            if (data.name.empty()) [[unlikely]]
            {
                throw GenerationException(
                    "Can't process generation of the header file due to unexpected empty the "
                    "enum's class name.");
            }

            auto endLines = std::count(result.rbegin(), result.rend(), '\n');
            while (!result.empty() && endLines < 2)
            {
                ++endLines;
                result.push_back('\n');
            }

            auto structName = "template<>\nstruct "s + namespaceName.data() + "<" + data.fullNamePath() + ">";

            result += structName;
            result += "\n{";
            result += generateSources(data, config);
            result += "\n}; // " + structName + "\n\n";
        }

        return result;
    }

    void EnumClassReflector::onScan(const FileData& fileData)
    {
        static const auto triggeredKeyword = getTriggerKeyword();
        const auto& content = fileData.getContent();

        for (const auto& token : _tokens)
        {
            token.requireValidTokenBasedOnContent(content);

            const char* p = content.c_str() + token.begin;
            const char* prevP = p;

            // Validating token definition
            if (strncmp(p, triggeredKeyword.data(), triggeredKeyword.size()) != 0) [[unlikely]]
            {
                throw SyntaxException("Invalid keyword was found. But expected: "
                                          + std::string(getTriggerKeyword()),
                                      prevP - content.c_str());
            }

            p = FindWordOnThisLine(p, "enum class");
            if (!p)
            {
                p = prevP;
                const char* startPtr = p;

                p = GoToNextLine(p);
                if (!p)
                {
                    WarnMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                std::string(getTriggerKeyword())
                                    + " keyword found, but 'enum class' wasn't found after it.");
                    continue;
                }

                prevP = p;
                p = FindWordOnThisLine(p, "enum class");
                if (!p)
                {
                    WarnMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                std::string(getTriggerKeyword())
                                    + " keyword found, but 'enum class' wasn't found after it.");
                    continue;
                }
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
                if (scope->attribute & Scope::Attr_Template)
                {
                    WarnMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                                "The current version of JRM can't process 'enum class' inside a "
                                "template scope: \""
                                    + PrettyPrintIdentifier(scope) + "\"");
                    continue;
                }
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
                           && (IsSpace(nameAndValue.second.back())
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

    std::string EnumClassReflector::generateSources(const TokenData& data,
                                                    const Config& config) const
    {
        std::string finalString = R"(
    @@FUNC_PREF_constexpr std::string_view Name() { return "@@ONLY_NAME_"; }
    @@FUNC_PREF_constexpr std::size_t Size() { return @@COUNT_; }
    @@FUNC_PREF_constexpr std::string_view ParentScope() { return "@@PARENTS_"; }

    @@FUNC_PREF_std::string_view ToString(::@@NAME_ value)
    {
        const auto& data = @@NAMESPACE_<@@NAME_>::ToMapCN();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        static const std::string_view empty{};
        return empty;
    }

    @@FUNC_PREF_std::optional<::@@NAME_> FromString(std::string_view value)
    {
        const auto& data = @@NAMESPACE_<@@NAME_>::ToMapNC();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        return std::nullopt;
    }

    @@FUNC_PREF_constexpr const std::array<::@@NAME_, @@COUNT_>& ToArrayC()
    {
        static constexpr std::array<::@@NAME_, @@COUNT_> constants = {
@@TO_ARRAY_C_
        };

        return constants;
    }

    @@FUNC_PREF_constexpr const std::array<std::string_view, @@COUNT_>& ToArrayN()
    {
        static constexpr std::array<std::string_view, @@COUNT_> names = {
@@TO_ARRAY_N_
        };

        return names;
    }

    @@FUNC_PREF_const std::unordered_map<::@@NAME_, std::string_view>& ToMapCN()
    {
        static const std::unordered_map<::@@NAME_, std::string_view> map = {
@@TO_ARRAY_CN_
        };

        return map;
    }

    @@FUNC_PREF_const std::unordered_map<std::string_view, ::@@NAME_>& ToMapNC()
    {
        static const std::unordered_map<std::string_view, ::@@NAME_> map = {
@@TO_ARRAY_NC_
        };

        return map;
    })";

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
                if (IsNewLine(str.back())) [[likely]]
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
                str += "\t\t\t\tstd::string_view(\"";
                str += name;
                str += "\"),\n";
            }

            if (!str.empty())
            {
                if (IsNewLine(str.back())) [[likely]]
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
                if (IsNewLine(str.back())) [[likely]]
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
                if (IsNewLine(str.back())) [[likely]]
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
        FindAndReplaceAll(finalString, onlyNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, countMark, std::to_string(data.constants.size()));
        FindAndReplaceAll(finalString, namespaceMark, BaseReflector::namespaceName.data());
        FindAndReplaceAll(finalString, funcPrefMark, "static ");

        return finalString;
    }

} // namespace JRM