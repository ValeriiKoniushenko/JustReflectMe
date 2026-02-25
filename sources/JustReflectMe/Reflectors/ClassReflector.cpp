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
        out.emplace("vector");
        out.emplace("string_view");
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
                    "class's name.");
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

            p = FindWordOnThisLine(p, "class");
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
                                "template scope: '"
                                    + PrettyPrintIdentifier(scope) + "'");
                    continue;
                }
                data.parentSpace = PrettyPrintScope(scope);
            }

            // Finding '{', but should check for ';'
            prevP = p;
            while (p && *p)
            {
                if (*p == '{')
                {
                    break;
                }

                if (*p == ';')
                {
                    WarnMessage(
                        content.c_str(), p - content.c_str(), fileData.getPath(),
                        "Expected class definition. But was found the forward declaration of the class: '"s
                            + data.name + "'");
                    continue;
                }

                ++p;
            }

            if (!p || *p != '{') [[unlikely]]
            {
                WarnMessage(content.c_str(), (p ? p - content.c_str() : 0), fileData.getPath(),
                            "Can't parse the class: '"s + data.name
                                + "' due to overloaded syntax or syntax errors.");
                continue;
            }

            const Scope* classScope = fileData.getScopes().getScopeAt(p);
            if (!classScope || !classScope->isValid() || classScope->type != Scope::Type::Class)
                [[unlikely]]
            {
                WarnMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                            "Can't parse scopes of the class: '"s + data.name
                                + "' due to overloaded syntax or syntax errors.");
                continue;
            }

            {
                // Validation for R_FRIEND();
                std::string_view src(classScope->start, classScope->end - classScope->start + 1);
                if (src.find("R_FRIEND") == std::string_view::npos)
                {
                    WarnMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                            "Can't parse the class: '"s + data.name
                                + "' - was skipped `R_FRIEND(" + data.name +");` in the class's scope. The absence of R_FRIEND won't able to generate reflective code in the correct way.");
                    continue;
                }
            }

            processFields(classScope, fileData, data);

            _data.emplace(token, std::move(data));
        }
    }

    void ClassReflector::processFields(const Scope* classScope, const FileData& fileData,
                                       TokenData& data)
    {
        static std::string_view fieldKeyword = "FIELD";
        std::vector<const char*> fields;

        const char* it = classScope->start;
        it = std::search(it, classScope->end, fieldKeyword.data(),
                         fieldKeyword.data() + fieldKeyword.size());

        while (it && it != classScope->end)
        {
            if (FileNavigator::IsWord(classScope->start, fieldKeyword, it - classScope->start))
            {
                if ((it = FindOnThisLine(it, ";")))
                {
                    if ((it = SkipAllBlanks(++it)))
                    {
                        fields.emplace_back(it);
                    }
                }
            }

            it = std::search(it, classScope->end, fieldKeyword.data(),
                             fieldKeyword.data() + fieldKeyword.size());
        }

        for (const char* p : fields)
        {
            FieldData field;
            int typenameReadOffset = 0;
            field.type = ReadAsTypename(p, typenameReadOffset);
            if (field.type.name.empty())
            {
                WarnMessage(p, p - fileData.getContent().c_str(), fileData.getPath(),
                            "Can't parse field of the class: '" + data.name
                                + "'. Can't detect typename.");
                continue;
            }

            p += typenameReadOffset;
            if (!std::isspace(*p))
            {
                WarnMessage(p, p - fileData.getContent().c_str(), fileData.getPath(),
                            "Can't parse field of the class: '" + data.name
                                + "'. Can't detect identifier.");
                continue;
            }

            p = SkipAllBlanks(p);

            field.name = ReadAsIdentifier(p);
            if (field.name.empty())
            {
                WarnMessage(p, p - fileData.getContent().c_str(), fileData.getPath(),
                            "Can't parse field of the class: '" + data.name
                                + "'. Can't detect field name.");
            }
            p += field.name.size();

            p = SkipAllBlanks(p);
            const char* const nameEnd = p;
            if (*p != ';')
            {
                if (*p == '=')
                {
                    p = SkipAllBlanks(p + 1);
                }

                if (*p == '{')
                {
                    auto s = classScope->findDeepest(p);
                    if (!s || !s->isValid())
                    {
                        WarnMessage(p, p - fileData.getContent().c_str(), fileData.getPath(),
                                    "Can't parse field of the class: '" + data.name
                                        + "'. Can't detect initializer.");
                        continue;
                    }

                    field.defaultValue = std::string(p, s->end - p + 1);
                }
                else
                {
                    const auto* end = strchr(p, ';');
                    if (!end)
                    {
                        WarnMessage(p, p - fileData.getContent().c_str(), fileData.getPath(),
                                    "Can't parse field of the class: '" + data.name
                                        + "'. Can't detect initializer(2).");
                        continue;
                    }

                    field.defaultValue = std::string(p, end - p);
                }

                while (!field.defaultValue.empty() && std::isspace(field.defaultValue.back()))
                {
                    field.defaultValue.pop_back();
                }

                if (field.defaultValue.contains(PostProcessedFile::stringPlaceholder))
                {
                    const char* iter = nameEnd;
                    while (iter && *iter && *iter != PostProcessedFile::stringPlaceholder)
                    {
                        ++iter;
                    }
                    if (iter && *iter == PostProcessedFile::stringPlaceholder)
                    {
                        auto str = "\""
                                   + fileData.getRealStringFromPlaceholderPos(
                                       iter - fileData.getContent().data() + 1)
                                   + "\"";

                        int len = 0;
                        while (*(iter + len) == PostProcessedFile::stringPlaceholder)
                        {
                            ++len;
                        }

                        auto start = field.defaultValue.find(PostProcessedFile::stringPlaceholder);
                        field.defaultValue.replace(start, len, str);
                    }
                }

                if (field.defaultValue.contains(PostProcessedFile::charPlaceholder))
                {
                    const char* iter = nameEnd;
                    while (iter && *iter && *iter != PostProcessedFile::charPlaceholder)
                    {
                        ++iter;
                    }
                    if (iter && *iter == PostProcessedFile::charPlaceholder)
                    {
                        auto diff = iter - fileData.getContent().data();
                        auto str = "'" + fileData.getRealCharFromPlaceholderPos(diff) + "'";

                        int len = 0;
                        while (*(iter + len) == PostProcessedFile::charPlaceholder)
                        {
                            ++len;
                        }

                        auto start = field.defaultValue.find(PostProcessedFile::charPlaceholder);
                        field.defaultValue.replace(start, len, str);
                    }
                }
            }

            data.fields.emplace_back(std::move(field));
        }
    }

    std::string ClassReflector::generateSources(const TokenData& data, const Config& config) const
    {
        std::string finalString = R"(
    @@FUNC_PREF_constexpr std::string_view Name() { return "@@ONLY_NAME_"; }
    @@FUNC_PREF_constexpr std::string_view ParentScope() { return "@@PARENTS_"; }
    @@FUNC_PREF_constexpr std::size_t GetFieldNumbers() { return @@FIELD_NUMBERS_; }
    @@FUNC_PREF_constexpr std::vector<RClassField> GetFields() {
        @@F_GET_FIELDS_
    })";

        // Default find & replace
        FindAndReplaceAll(finalString, nameMark, data.fullNamePath());
        FindAndReplaceAll(finalString, onlyNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, namespaceMark, BaseReflector::namespaceName.data());
        FindAndReplaceAll(finalString, funcPrefMark, "static ");

        // Class-specific find & replace
        FindAndReplaceAll(finalString, fieldNumbers, std::to_string(data.fields.size()));

        // =================== F_GET_FIELDS_ =========================
        {
            std::string out;
            out += "return {\n";
            for (const auto& field : data.fields)
            {
                out += "\t\t\t{ \"" + field.type.getNameWithCV() + "\", \"" + field.name
                       + "\" },\n";
            }
            out += "\t\t};";
            FindAndReplaceAll(finalString, "@@F_GET_FIELDS_", out);
        }

        return finalString;
    }

} // namespace JRM
