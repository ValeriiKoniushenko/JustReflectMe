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

#include "ClassReflector.h"

#include "../Config.h"
#include "JustReflectMe/FileData.h"
#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/StringHelper.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace FileNavigator;
using namespace StringHelper;
using namespace std::string_literals;

namespace JRM
{

    std::set<std::string> ClassReflector::getIncludes() const
    {
        auto out = BaseReflector::getIncludes();
        out.emplace("optional");
        out.emplace("unordered_map");
        out.emplace("array");
        return out;
    }

    std::string ClassReflector::onGenerateHeaderFile(FileData& fileData, const Config& config) const
    {
        std::string result;
        result.reserve(1024 * 4);

        for (const auto& [_, data] : _data)
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

            auto structName = "struct "s + namespaceName.data() + "<" + data.fullNamePath() + ">";

            result += "template<>\n";
            result += structName;
            result += "\n{";
            result += generateSources(data, config);
            result += "\n}; // " + structName;
        }

        return result;
    }

    void ClassReflector::onScan(const FileData& fileData)
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

            p = FindWordOnThisLine(p, "enum");
            if (!p)
            {
                p = prevP;
                const char* startPtr = p;

                p = GoToNextLine(p);
                if (!p)
                {
                    WarnMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                std::string(getTriggerKeyword())
                                    + " keyword found, but 'class' wasn't found after it.");
                    continue;
                }

                prevP = p;
                p = FindWordOnThisLine(p, "class");
                if (!p)
                {
                    WarnMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                std::string(getTriggerKeyword())
                                    + " keyword found, but 'class' wasn't found after it.");
                    continue;
                }
            }

            static const auto classLength = strlen("class");
            p += classLength;

            TokenData data;
            data.name = ReadAsIdentifier(p);
            if (data.name.empty())
            {
                throw SyntaxException("Not found class's identifier.", p - content.c_str());
            }

            if (const Scope* scope = fileData.getScopes().getScopeAt(p))
            {
                if (scope->attribute & Scope::Attr_Template)
                {
                    WarnMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                                "The current version of JRM can't process 'class' inside a "
                                "template scope: \""
                                    + PrettyPrintIdentifier(scope) + "\"");
                    continue;
                }
                data.parentSpace = PrettyPrintScope(scope);
            }

            _data.emplace(token, std::move(data));
        }
    }

    std::string ClassReflector::generateSources(const TokenData& data, const Config& config) const
    {
        std::string finalString = R"(
    @@FUNC_PREF_std::string_view Name() { return "@@ONLY_NAME_"; }
    @@FUNC_PREF_std::string_view ParentScope() { return "@@PARENTS_"; }
        )";

        FindAndReplaceAll(finalString, nameMark, data.fullNamePath());
        FindAndReplaceAll(finalString, onlyNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, namespaceMark, BaseReflector::namespaceName.data());
        FindAndReplaceAll(finalString, funcPrefMark, "static ");

        return finalString;
    }

} // namespace JRM