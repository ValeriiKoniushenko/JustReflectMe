// JustReflectMe
// Copyright 2018-2026 Valerii Koniushenko
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0

#include "ClassReflector.h"

#include "../Adapter.h"
#include "../Config.h"
#include "JustReflectMe/FileData.h"
#include "JustReflectMe/FileNavigationHelper.h"
#include "JustReflectMe/Reflectors/BaseReflector.h"
#include "JustReflectMe/StringHelper.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>

using namespace FileNavigator;
using namespace StringHelper;
using namespace std::string_literals;

namespace JRM
{

    std::optional<TypeMeta> ClassReflector::findKnownTypeMeta(const std::string& fullPath) const
    {
        std::optional<TypeMeta> out;
        int counter = 0;
        for (const auto& data : _data | std::views::values)
        {
            if (data.fullNamePath() == fullPath)
            {
                out = TypeMeta{};
                out->type = ContextType::Class;

                ++counter;
            }
        }

        if (counter > 1)
        {
            std::cout << "[JustReflectMe] ClassReflector | Warning: found more than one typename '"
                      << fullPath << std::endl;
        }

        return counter == 1 ? out : std::nullopt;
    }

    void ClassReflector::postScanCrossLinksResolving()
    {
        // Fields resolution between all registered types
        for (auto& token : _data | std::views::values)
        {
            for (auto& field : token.fields)
            {
                field.typeMeta = findGloballyKnownTypeMeta(field.type.name);
            }
        }
    }

    std::set<std::string> ClassReflector::getIncludes() const
    {
        auto out = BaseReflector::getIncludes();
        out.emplace("array");
        out.emplace("concepts");
        out.emplace("type_traits");
        out.emplace("vector");
        out.emplace("string_view");
        return out;
    }

    std::string ClassReflector::onGenerateHeaderFile(FileData&) const
    {
        std::string result;
        result.reserve(1024 * 4);

        std::map<TokenEntry, const TokenData*, std::less<TokenEntry>> reverseSortedData;
        for (const auto& [_, data] : _data)
        {
            reverseSortedData.emplace(_, &data);
        }

        for (const auto& [_, data] : reverseSortedData)
        {
            if (data->name.empty()) [[unlikely]]
            {
                throw GenerationException(
                    "Can't process generation of the header file due to unexpected empty the "
                    "class's name.");
            }

            auto endLines = std::count_if(result.rbegin(), result.rend(),
                                          [](auto ch) { return IsNewLine(ch); });

            while (!result.empty() && endLines < 2)
            {
                ++endLines;
                result.push_back('\n');
            }

            auto structName = "struct "s + namespaceName.data() + "<" + data->fullNamePath() + ">";

            if (!result.empty() && result.back() != '\n')
            {
                result += "\n\n";
            }

            result += "template<>\n";
            result += structName;
            result += "\n{";
            result += generateSources(*data);
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

            bool trueIfIsClass = true;
            p = FindWordOnThisLine(p, "class");
            if (!p)
            {
                p = prevP;
                p = FindWordOnThisLine(p, "struct");
                trueIfIsClass = false;
            }
            if (!p)
            {
                p = prevP;
                const char* startPtr = p;

                p = SkipAllBlanks(p);
                if (strncmp(p, triggeredKeyword.data(), triggeredKeyword.size()) != 0) [[unlikely]]
                {
                    errorMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                 "Internal error. Can't find a 'CLASS'");
                    continue;
                }
                p += triggeredKeyword.size();
                p = SkipAllBlanks(p);
                if (!p || *p == '\0' || *p != '(')
                {
                    errorMessage(
                        content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                        "Can't find a '(' after the 'CLASS' keyword. Expected: 'CLASS( ... )'.");
                    continue;
                }

                p = FindScopeEnd(p);
                if (!p) [[unlikely]]
                {
                    errorMessage(
                        content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                        "Can't find a ')' after the 'CLASS' keyword. Expected: 'CLASS( ... )'.");
                    continue;
                }

                p = SkipAllBlanks(++p);
                if (*p == ';')
                {
                    ++p;
                }
                p = SkipAllBlanks(p);

                prevP = p;
                p = FindWordOnThisLine(p, "class");
                if (!p)
                {
                    p = prevP;
                    p = FindWordOnThisLine(p, "struct");
                    trueIfIsClass = false;
                }
                if (!p)
                {
                    errorMessage(content.c_str(), startPtr - content.c_str(), fileData.getPath(),
                                 std::string(getTriggerKeyword())
                                     + " keyword found, but 'class' wasn't found after it.");
                    continue;
                }
            }

            static const auto classLength = strlen(trueIfIsClass ? "class" : "struct");
            p += classLength;

            TokenData data;

            // try to read attributes: [[smth, smth]]
            p = SkipAllBlanks(p);
            if (StartWith(p, "[["))
            {
                p += 2;
                p = SkipAllBlanks(p);

                const auto* end = strstr(p, "]]");
                if (!end)
                {
                    throw SyntaxException("Can't parse the attribute specifier[s] of the class.",
                                          p - content.c_str());
                }

                data.attribs = SplitString(std::string(p, end - p));

                p = end + 2;
            }
            p = SkipAllBlanks(p);

            data.name = ReadAsIdentifier(p);
            if (data.name.empty())
            {
                throw SyntaxException("Not found class's identifier.", p - content.c_str());
            }

            if (const Scope* scope = fileData.getScopes().getScopeAt(p))
            {
                if (scope->attribute & Scope::Attr_Template)
                {
                    errorMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                                 "The current version of JRM can't process 'class' inside a "
                                 "template scope: '"
                                     + PrettyPrintIdentifier(scope) + "'");
                    continue;
                }
                data.parentSpace = PrettyPrintScope(scope);
            }

            // Scanning parents
            p = GoToNotSpace(p);
            p += data.name.size();
            p = GoToNotSpace(p);
            if (p[0] == ':' && p[1] != ':')
            {
                // Scanning parents

                do
                {
                    p = GoToNotSpace(p + 1);
                    if (StartWith(p, "public") || StartWith(p, "protected")
                        || StartWith(p, "private"))
                    {
                        p = GoToBlank(p);
                        p = GoToNotSpace(p);
                    }

                    int offset = 0;
                    data.parents.push_back(ReadAsTypename(p, offset).getNameWithCV());
                    p += offset;
                    p = SkipAllBlanks(p);
                } while (*p && *p == ',');
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
                    errorMessage(
                        content.c_str(), p - content.c_str(), fileData.getPath(),
                        "Expected class definition. But was found the forward declaration of the class: '"s
                            + data.name + "'");
                    continue;
                }

                ++p;
            }

            if (!p || *p != '{') [[unlikely]]
            {
                errorMessage(content.c_str(), (p ? p - content.c_str() : 0), fileData.getPath(),
                             "Can't parse the class: '"s + data.name
                                 + "' due to unexpected syntax or syntax errors.");
                continue;
            }

            const Scope* classScope = fileData.getScopes().getScopeAt(p);
            if (!classScope || !classScope->isValid()
                || (classScope->type != ContextType::Class
                    && classScope->type != ContextType::Struct)) [[unlikely]]
            {
                errorMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                             "Can't parse scopes of the class: '"s + data.name
                                 + "' due to unexpected syntax or syntax errors.");
                continue;
            }

            {
                // Validation for R_FRIEND();
                std::string_view src(classScope->start, classScope->end - classScope->start + 1);
                std::size_t pos = std::string_view::npos;
                std::string rFriend;
                for (auto val : _config->rFriendAliases->value)
                {
                    pos = src.find(val + "(");
                    if (pos != std::string_view::npos)
                    {
                        rFriend = val;
                        break;
                    }
                }

                if (pos == std::string_view::npos)
                {
                    errorMessage(content.c_str(), p - content.c_str(), fileData.getPath(),
                            "Can't parse the class: '"s + data.name
                                + "' - was skipped `R_FRIEND(" + data.name +");` in the class's scope. The absence of R_FRIEND won't able to generate reflective code in the correct way.");
                    continue;
                }

                const char* bracketPos = classScope->start + pos + rFriend.size();
                if (FileNavigator::IsSpace(*bracketPos))
                {
                    bracketPos = FileNavigator::SkipAllBlanks(bracketPos);
                }

                if (*bracketPos != '(')
                {
                    static std::string_view rFriendDecl = "_DECL";
                    if (FileNavigator::StartWith(bracketPos, rFriendDecl))
                    {
                        bracketPos += rFriendDecl.size();
                        if (FileNavigator::IsSpace(*bracketPos))
                        {
                            bracketPos = FileNavigator::SkipAllBlanks(bracketPos);
                        }
                    }
                }

                if (*bracketPos != '(')
                {
                    errorMessage(
                        content.c_str(), p - content.c_str(), fileData.getPath(),
                        "Can't parse the class: '"s + data.name + "' - `R_FRIEND(" + data.name
                            + ");` in the class's scope. Internal error or syntax is too complex.");
                    continue;
                }

                const auto bracketEnd = FileNavigator::FindScopeEnd(bracketPos);
                if (!bracketEnd) [[unlikely]]
                {
                    errorMessage(
                        content.c_str(), p - content.c_str(), fileData.getPath(),
                        "Can't parse the class: '"s + data.name + "' - `R_FRIEND(" + data.name
                            + ");` in the class's scope. Internal error(2) or syntax is too complex.");
                    continue;
                }

                bracketPos = FileNavigator::SkipAllBlanks(++bracketPos);

                bool isFirst = true;
                while (bracketPos < bracketEnd)
                {
                    int offset = 0;
                    const auto type = FileNavigator::ReadAsTypename(bracketPos, offset);
                    bracketPos += std::max(offset, 1); // un-infinity guard

                    bracketPos = SkipAllBlanks(bracketPos);
                    if (*bracketPos == ',')
                    {
                        ++bracketPos;
                    }
                    bracketPos = SkipAllBlanks(bracketPos);

                    if (!isFirst)
                    {
                        data.serializableParents.emplace_back(type.name);
                    }

                    isFirst = false;
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
        const char* const content = fileData.getContent().c_str();

        struct FieldMeta
        {
            const char* p = nullptr;
            int genFlags = 0;
            int flags = RPoint::Default;
            std::vector<std::string> attribs;
            std::string min;
            std::string max;
            std::string step;
            bool suppressLimits = false;
            MetaEntries meta;
            std::string setter;
        };
        std::vector<FieldMeta> fields;

        const char* it = classScope->start;
        it = std::search(it, classScope->end, fieldKeyword.data(),
                         fieldKeyword.data() + fieldKeyword.size());

        while (it && it != classScope->end)
        {
            if (FileNavigator::IsWord(classScope->start, fieldKeyword, it - classScope->start))
            {
                FieldMeta meta;

                it += fieldKeyword.size();
                it = SkipAllBlanks(it);
                if (*it != '(') [[unlikely]]
                {
                    errorMessage(content, it - content, fileData.getPath(),
                                 "Can't parse field of the class: '" + data.name
                                     + "'. Can't validate the field's structure.");
                    continue;
                }

                const auto* fieldClose = FileNavigator::FindScopeEnd(it);
                ++it;
                while (it < fieldClose)
                {
                    int offset = 0;
                    while (it < fieldClose && (*it == ',' || std::isspace(*it)))
                    {
                        ++it;
                    }

                    const auto type = FileNavigator::ReadAsTypename(it, offset);
                    it += std::max(offset, 1);

                    if (type.name == "R::NoDefaultValue")
                    {
                        meta.genFlags |= static_cast<int>(RPoint::NoDefaultValue);
                    }
                    else if (type.name == "R::NonRequired")
                    {
                        meta.flags |= RPoint::NonRequired;
                    }
                    else if (type.name == "R::Required")
                    {
                        meta.flags |= RPoint::Required;
                    }
                    else if (type.name == "R::Default")
                    {
                        meta.flags |= RPoint::Default;
                    }
                    else if (type.name == "R::Attr")
                    {
                        meta.flags |= RPoint::Attr;

                        // expecting: = SomeValue
                        it = SkipAllBlanks(it);
                        if (it >= fieldClose || *it != '=')
                        {
                            errorMessage(content, it - content, fileData.getPath(),
                                         "Can't parse field of the class: '" + data.name
                                             + "'. Can't validate the field's attribute 'Attr'.");
                            continue;
                        }

                        it = SkipAllBlanks(it + 1);

                        if (it >= fieldClose || *it == '=')
                        {
                            errorMessage(
                                content, it - content, fileData.getPath(),
                                "Can't parse field of the class: '" + data.name
                                    + "'. Can't validate the field's attribute 'Attr'. (2)");
                            continue;
                        }

                        it = SkipAllBlanks(it);
                        auto attr = ReadAsIdentifier(it);
                        if (attr.empty())
                        {
                            errorMessage(content, it - content, fileData.getPath(),
                                         "Can't parse field of the class: '" + data.name
                                             + "'. Can't validate the field's 'Attr' value.");
                            continue;
                        }

                        it += attr.size();

                        meta.attribs.push_back(std::move(attr));
                    }
                    else if (type.name == "R::Min")
                    {
                        meta.flags |= RPoint::Min;

                        // expecting: = SomeValue
                        it = SkipAllBlanks(it);
                        if (it >= fieldClose || *it != '=')
                        {
                            errorMessage(content, it - content, fileData.getPath(),
                                         "Can't parse field of the class: '" + data.name
                                             + "'. Can't validate the field's attribute 'Min'.");
                            continue;
                        }

                        it = SkipAllBlanks(it + 1);

                        if (!std::isdigit(*it) && *it != '.' && *it != '-')
                        {
                            errorMessage(
                                content, it - content, fileData.getPath(),
                                "Can't parse field of the class: '" + data.name
                                    + "'. Can't validate the value of the field's attribute "
                                      "'Min'.");
                            continue;
                        }

                        if (TryParseNumber(it).has_value())
                        {
                            meta.min = {};
                            while (std::isdigit(*it) || *it == '.' || *it == '\'' || *it == '-')
                            {
                                meta.min += *it;
                                ++it;
                            }
                        }
                    }
                    else if (type.name == "R::Max")
                    {
                        meta.flags |= RPoint::Max;

                        // expecting: = SomeValue
                        it = SkipAllBlanks(it);
                        if (it >= fieldClose || *it != '=')
                        {
                            errorMessage(content, it - content, fileData.getPath(),
                                         "Can't parse field of the class: '" + data.name
                                             + "'. Can't validate the field's attribute 'Max'.");
                            continue;
                        }

                        it = SkipAllBlanks(it + 1);

                        if (!std::isdigit(*it) && *it != '.' && *it != '-')
                        {
                            errorMessage(
                                content, it - content, fileData.getPath(),
                                "Can't parse field of the class: '" + data.name
                                    + "'. Can't validate the value of the field's attribute "
                                      "'Max'.");
                            continue;
                        }

                        if (TryParseNumber(it).has_value())
                        {
                            meta.max = {};
                            while (std::isdigit(*it) || *it == '.' || *it == '\'' || *it == '-')
                            {
                                meta.max += *it;
                                ++it;
                            }
                        }
                    }
                    else if (type.name == "R::Step")
                    {
                        meta.flags |= RPoint::Step;

                        // expecting: = SomeValue
                        it = SkipAllBlanks(it);
                        if (it >= fieldClose || *it != '=')
                        {
                            errorMessage(content, it - content, fileData.getPath(),
                                         "Can't parse field of the class: '" + data.name
                                             + "'. Can't validate the field's attribute 'Step'.");
                            continue;
                        }

                        it = SkipAllBlanks(it + 1);

                        if (!std::isdigit(*it) && *it != '.')
                        {
                            errorMessage(
                                content, it - content, fileData.getPath(),
                                "Can't parse field of the class: '" + data.name
                                    + "'. Can't validate the value of the field's attribute "
                                      "'Step'.");
                            continue;
                        }

                        if (TryParseNumber(it).has_value())
                        {
                            meta.step = {};
                            while (std::isdigit(*it) || *it == '.' || *it == '\'')
                            {
                                meta.step += *it;
                                ++it;
                            }
                        }
                    }
                    else if (type.name == "R::SuppressLimits")
                    {
                        meta.flags |= RPoint::SuppressLimits;
                    }
                    else if (type.name == "R::Meta")
                    {
                        meta.flags |= RPoint::Meta;
                        parseMetaGroup(it, fieldClose, fileData, data.name, meta.meta,
                                       meta.setter);
                    }
                    else
                    {
                        if (!type.name.empty()) // suppress no-type warnings
                        {
                            warnMessage(content, it - content, fileData.getPath(),
                                        "The FIELD's parameter or expression is not recognized: '"
                                            + type.name + "'");
                        }

                        // Skipping one token
                        it = std::strpbrk(it, ",);");
                        if (!it || it >= fieldClose)
                        {
                            it = fieldClose;
                        }
                    }
                }

                if ((it = FindOnThisLine(fieldClose, ";")))
                {
                    if ((it = SkipAllBlanks(++it)))
                    {
                        meta.p = it;
                        fields.emplace_back(meta);
                    }
                }
            }

            it = std::search(it, classScope->end, fieldKeyword.data(),
                             fieldKeyword.data() + fieldKeyword.size());
        }

        for (auto&& fieldMeta : fields)
        {
            const auto* p = fieldMeta.p;
            FieldData field;
            field.flags = fieldMeta.flags;
            field.allFlags = static_cast<uint64_t>(fieldMeta.flags)
                             | static_cast<uint64_t>(fieldMeta.genFlags);
            field.meta = std::move(fieldMeta.meta);
            field.setter = std::move(fieldMeta.setter);

            int typenameReadOffset = 0;
            field.type = ReadAsTypename(p, typenameReadOffset);
            if (field.type.name.empty())
            {
                errorMessage(content, p - content, fileData.getPath(),
                             "Can't parse field of the class: '" + data.name
                                 + "'. Can't detect typename.");
                continue;
            }

            p += typenameReadOffset;
            if (!IsSpace(*p))
            {
                errorMessage(content, p - content, fileData.getPath(),
                             "Can't parse field of the class: '" + data.name
                                 + "'. Can't detect identifier.");
                continue;
            }

            p = SkipAllBlanks(p);

            field.name = ReadAsIdentifier(p);
            if (field.name.empty())
            {
                warnMessage(content, p - content, fileData.getPath(),
                            "Can't parse field of the class: '" + data.name
                                + "'. Can't detect field name.");
            }

            p += field.name.size();

            p = SkipAllBlanks(p);
            const char* const nameEnd = p;
            if (*p != ';' && (fieldMeta.genFlags & static_cast<int>(RPoint::NoDefaultValue)) == 0)
            {
                if (*p == '=')
                {
                    p = SkipAllBlanks(p + 1);
                }

                if (*p == '{')
                {
                    const auto* s = classScope->findDeepest(p);
                    if (!s || !s->isValid())
                    {
                        errorMessage(content, p - content, fileData.getPath(),
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
                        errorMessage(content, p - content, fileData.getPath(),
                                     "Can't parse field of the class: '" + data.name
                                         + "'. Can't detect initializer(2).");
                        continue;
                    }

                    field.defaultValue = std::string(p, end - p);
                }

                while (!field.defaultValue.empty() && IsSpace(field.defaultValue.back()))
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

            field.attributes = std::move(fieldMeta.attribs);

            field.min = fieldMeta.min;
            if (!field.min.empty() && !IsNumericTypeName(field.type.name))
            {
                warnMessage(content, fieldMeta.p - content, fileData.getPath(),
                            std::format("Parse warning for field '{} {}': Only numeric types can "
                                        "be marked with R::Min",
                                        field.type.getNameWithCV(), field.name));
            }

            field.max = fieldMeta.max;
            if (!field.max.empty() && !IsNumericTypeName(field.type.name))
            {
                warnMessage(content, fieldMeta.p - content, fileData.getPath(),
                            std::format("Parse warning for field '{} {}': Only numeric types can "
                                        "be marked with R::Max",
                                        field.type.getNameWithCV(), field.name));
            }

            field.step = fieldMeta.step;
            if (!field.step.empty() && !IsNumericTypeName(field.type.name))
            {
                warnMessage(content, fieldMeta.p - content, fileData.getPath(),
                            std::format("Parse warning for field '{} {}': Only numeric types can "
                                        "be marked with R::Step",
                                        field.type.getNameWithCV(), field.name));
            }

            data.fields.emplace_back(std::move(field));
        }
    }

    namespace
    {
        bool IsIdentifierStart(char ch)
        {
            return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
        }

        bool IsIdentifierChar(char ch)
        {
            return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
        }

        /// Reads `name` or `a::b::name`; leaves `it` past it. Empty if `it` is not at one.
        std::string ReadQualifiedIdentifier(const char*& it, const char* end)
        {
            std::string out;
            while (it < end && IsIdentifierStart(*it))
            {
                while (it < end && IsIdentifierChar(*it))
                {
                    out += *it++;
                }
                if (it + 1 < end && it[0] == ':' && it[1] == ':' && IsIdentifierStart(it[2]))
                {
                    out += "::";
                    it += 2;
                    continue;
                }
                break;
            }
            return out;
        }

        /// Reads a numeric literal such as `3`, `-2.5` or `1'000`; leaves `it` past it.
        std::string ReadNumber(const char*& it, const char* end)
        {
            std::string out;
            if (it < end && *it == '-')
            {
                out += *it++;
            }
            while (it < end
                   && (std::isdigit(static_cast<unsigned char>(*it)) != 0 || *it == '.'
                       || *it == '\''))
            {
                out += *it++;
            }
            return out;
        }

        void SkipToNextMetaEntry(const char*& it, const char* groupClose)
        {
            while (it < groupClose && *it != ',')
            {
                ++it;
            }
        }
    } // namespace

    void ClassReflector::parseMetaGroup(const char*& it, const char* fieldClose,
                                        const FileData& fileData, const std::string& className,
                                        MetaEntries& meta, std::string& setter)
    {
        const char* const content = fileData.getContent().c_str();
        const auto fail = [&](const char* where, const std::string& what)
        {
            errorMessage(content, where - content, fileData.getPath(),
                         "Can't parse field of the class: '" + className + "'. R::Meta: " + what);
        };

        it = SkipAllBlanks(it);
        if (it >= fieldClose || *it != '(')
        {
            fail(it, "expected '(' after 'R::Meta'. Expected: R::Meta(Key, Key = Value, ...).");
            it = fieldClose;
            return;
        }

        const char* const groupClose = FindScopeEnd(it);
        if (!groupClose || groupClose > fieldClose)
        {
            fail(it, "can't find the closing ')' of the group.");
            it = fieldClose;
            return;
        }

        ++it;
        while (it < groupClose)
        {
            while (it < groupClose && (*it == ',' || IsSpace(*it)))
            {
                ++it;
            }
            if (it >= groupClose)
            {
                break;
            }

            const char* const entryStart = it;
            std::string key = ReadQualifiedIdentifier(it, groupClose);
            if (key.empty() || key.contains("::"))
            {
                fail(entryStart, "expected a plain identifier as a key.");
                SkipToNextMetaEntry(it, groupClose);
                continue;
            }

            it = SkipAllBlanks(it);

            std::string value;
            bool hasValue = false;
            bool isIdentifierValue = false;
            if (it < groupClose && *it == '=')
            {
                it = SkipAllBlanks(it + 1);
                const char* const valueStart = it;

                if (it < groupClose && *it == PostProcessedFile::stringPlaceholder)
                {
                    const auto literal
                        = fileData.getStringLiteralAt(static_cast<std::size_t>(it - content));
                    while (it < groupClose && *it == PostProcessedFile::stringPlaceholder)
                    {
                        ++it;
                    }
                    if (!literal)
                    {
                        fail(valueStart, "can't restore the string value of '" + key + "'.");
                        SkipToNextMetaEntry(it, groupClose);
                        continue;
                    }
                    value = *literal;
                    hasValue = true;
                }
                else if (it < groupClose && IsIdentifierStart(*it))
                {
                    value = ReadQualifiedIdentifier(it, groupClose);
                    hasValue = true;
                    isIdentifierValue = true;
                }
                else if (it < groupClose
                         && (std::isdigit(static_cast<unsigned char>(*it)) != 0 || *it == '-'
                             || *it == '.'))
                {
                    value = ReadNumber(it, groupClose);
                    if (!TryParseNumber(value.c_str()).has_value())
                    {
                        fail(valueStart, "invalid numeric value of '" + key + "'.");
                        SkipToNextMetaEntry(it, groupClose);
                        continue;
                    }
                    hasValue = true;
                }
                else
                {
                    fail(valueStart, "missing value after '" + key
                                         + " ='. Expected an identifier, a number or a string.");
                    SkipToNextMetaEntry(it, groupClose);
                    continue;
                }
            }

            it = SkipAllBlanks(it);
            if (it < groupClose && *it != ',')
            {
                fail(it, "unexpected token after '" + key + "'. Entries are separated by ','.");
                SkipToNextMetaEntry(it, groupClose);
                continue;
            }

            if (key == "Setter")
            {
                if (!hasValue || !isIdentifierValue || value.contains("::"))
                {
                    fail(entryStart, "'Setter' expects a member function name: Setter = setFoo.");
                    continue;
                }
                setter = value;
            }

            if (auto existing = std::ranges::find(meta, key, &MetaEntries::value_type::first);
                existing != meta.end())
            {
                warnMessage(content, entryStart - content, fileData.getPath(),
                            "Field of the class '" + className + "': R::Meta key '" + key
                                + "' is repeated; the last value is used.");
                existing->second = std::move(value);
            }
            else
            {
                meta.emplace_back(std::move(key), std::move(value));
            }
        }

        it = groupClose + 1;
    }

    std::string ClassReflector::generateSources(const TokenData& data) const
    {
        std::string finalString = R"(
    @@FUNC_PREF_constexpr std::string_view Name() { return "@@ONLY_NAME_"; }
    @@FUNC_PREF_constexpr std::string_view FullName() { return "@@NAME_"; }
    @@FUNC_PREF_constexpr std::string_view ParentScope() { return "@@PARENTS_"; }
    @@FUNC_PREF_constexpr std::size_t GetFieldNumbers() { return @@FIELD_NUMBERS_; }
    @@FUNC_PREF_constexpr std::vector<RClassField> GetFields() {
        return @@F_FIELDS_INIT_;
    }
    @@FUNC_PREF_constexpr std::unordered_map<std::string, RClassField> GetFieldsMap()
    {
        return RInternal::GetClassFieldsAsMap(GetFields());
    }

    // Declaration order; built once, so element addresses are stable for the program's lifetime.
    @@FUNC_PREF_const std::array<RClassField, @@FIELD_NUMBERS_>& GetFieldsArray()
    {
        static const std::array<RClassField, @@FIELD_NUMBERS_> fields@@F_FIELDS_ARRAY_INIT_;
        return fields;
    }

    // Calls visitor(const RClassField&, FieldType&) per field in declaration order.
    // FieldType keeps the constness of `obj` and of the field declaration.
    template<class Self, class V>
        requires std::same_as<std::remove_const_t<Self>, @@NAME_>
    @@FUNC_PREF_void ForEachField([[maybe_unused]] Self& obj, [[maybe_unused]] V&& visitor)
    {
        [[maybe_unused]] const auto& fields = GetFieldsArray();
@@F_FOR_EACH_FIELD_
    }

    // Calls visitor(const RClassField&, std::integral_constant<FieldType T::*, &T::field>, setter)
    // per field in declaration order. `setter` is RNoSetter, or a captureless callable
    // void(T&, const FieldType&) generated from R::Meta(Setter = memberFunction).
    template<class V>
    @@FUNC_PREF_void ForEachMember([[maybe_unused]] V&& visitor)
    {
        [[maybe_unused]] const auto& fields = GetFieldsArray();
@@F_FOR_EACH_MEMBER_
    }

    @@FUNC_PREF_constexpr std::size_t GetBaseNumbers() { return @@BASE_NUMBERS_; }

    // Calls visitor(std::type_identity<Base>{}) per base listed in R_FRIEND, in that order.
    template<class V>
    @@FUNC_PREF_void ForEachBase([[maybe_unused]] V&& visitor)
    {
@@F_FOR_EACH_BASE_
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    @@FUNC_PREF_void Serialize(const @@NAME_& obj, RResourceStream<RImpl>& s, bool noSignals = false)
    {
        @@F_SERIALIZE_
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    [[nodiscard]] @@FUNC_PREF_RResourceStream<RImpl> Serialize(const @@NAME_& obj, bool noSignals = false)
    {
        RResourceStream<RImpl> s;
        Serialize(obj, s, noSignals);
        return s;
    }

    template<IsResourceStreamImpl RImpl = RJsonResourceStream>
    @@FUNC_PREF_void Deserialize(const RResourceStream<RImpl>& s, @@NAME_& obj, bool noSignals = false)
    {
        @@F_DESERIALIZE_
    }

    @@FUNC_PREF_bool GetField([[maybe_unused]] @@NAME_& obj, [[maybe_unused]] const char* fieldName, const std::function<void(void*, const char*)>& onFound)
    {
        if (!onFound) return false;
@@F_GET_FIELD_
    })";

        // Class-specific find & replace
        FindAndReplaceAll(finalString, fieldNumbers, std::to_string(data.fields.size()));

        // ============ F_FIELDS_INIT_ / F_FIELDS_ARRAY_INIT_ =============
        {
            std::string out;
            out += "{\n";
            for (const auto& field : data.fields)
            {
                // Buliding of RClassField::Attributes
                std::string attributesOut = "RClassField::Attributes{ ";

                // RClassField::Attributes::attribs
                {
                    attributesOut += "\n\t\t\t\t\t";
                    attributesOut += ".attribs = { ";
                    std::string attribOut;
                    for (auto&& attrib : field.attributes)
                    {
                        if (!attribOut.empty())
                        {
                            attribOut += ", ";
                        }
                        attribOut += "\"" + attrib + "\"";
                    }
                    attribOut += " },";

                    attributesOut += attribOut;
                }

                // RClassField::Attributes::min
                attributesOut += "\n\t\t\t\t\t";
                if (field.min.empty())
                {
                    attributesOut += ".min = std::nullopt,";
                }
                else
                {
                    attributesOut += ".min = JRM::Number{ " + field.min + " },";
                }

                // RClassField::Attributes::min
                attributesOut += "\n\t\t\t\t\t";
                if (field.max.empty())
                {
                    attributesOut += ".max = std::nullopt,";
                }
                else
                {
                    attributesOut += ".max = JRM::Number{ " + field.max + " },";
                }

                // RClassField::Attributes::step
                attributesOut += "\n\t\t\t\t\t";
                if (field.step.empty())
                {
                    attributesOut += ".step = std::nullopt,";
                }
                else
                {
                    attributesOut += ".step = JRM::Number{ " + field.step + " },";
                }

                // RClassField::Attributes::flags
                attributesOut += "\n\t\t\t\t\t";
                attributesOut += ".flags = " + std::to_string(field.allFlags) + ",";

                // RClassField::Attributes::meta
                attributesOut += "\n\t\t\t\t\t";
                if (field.meta.empty())
                {
                    attributesOut += ".meta = {},";
                }
                else
                {
                    attributesOut += ".meta = { ";
                    for (std::size_t i = 0; i < field.meta.size(); ++i)
                    {
                        if (i != 0)
                        {
                            attributesOut += ", ";
                        }
                        attributesOut += std::format("RMetaEntry{{ \"{}\", \"{}\" }}",
                                                     field.meta[i].first, field.meta[i].second);
                    }
                    attributesOut += " },";
                }

                attributesOut += "\n\t\t\t\t";
                attributesOut += "}";

                out += std::format(
                    "\t\t\t{{ \n\t\t\t\t\"{}\", \n\t\t\t\t\"{}\", \n\t\t\t\t{} \n\t\t\t}},\n",
                    field.type.getNameWithCV(), field.name, attributesOut);
            }
            out += "\t\t}";

            FindAndReplaceAll(finalString, "@@F_FIELDS_ARRAY_INIT_",
                              data.fields.empty() ? "{}"s : "{ " + out + " }");
            FindAndReplaceAll(finalString, "@@F_FIELDS_INIT_", out);
        }

        // =================== F_FOR_EACH_FIELD_ / F_FOR_EACH_MEMBER_ =========================
        {
            std::string forEachField;
            std::string forEachMember;
            for (std::size_t i = 0; i < data.fields.size(); ++i)
            {
                const auto& field = data.fields[i];
                const auto memberPath = data.fullNamePath() + "::" + field.name;

                forEachField += std::format("\t\tvisitor(fields[{}], obj.{});\n", i, field.name);

                std::string setter = "RNoSetter{}";
                if (!field.setter.empty())
                {
                    setter = std::format(
                        "[]({}& obj, const std::remove_cv_t<decltype({})>& value) {{ "
                        "obj.{}(value); }}",
                        data.fullNamePath(), memberPath, field.setter);
                }
                forEachMember += std::format(
                    "\t\tvisitor(fields[{}], std::integral_constant<decltype(&{}), &{}>{{}}, {});\n",
                    i, memberPath, memberPath, setter);
            }

            FindAndReplaceAll(finalString, "@@F_FOR_EACH_FIELD_", forEachField);
            FindAndReplaceAll(finalString, "@@F_FOR_EACH_MEMBER_", forEachMember);
        }

        // =================== F_FOR_EACH_BASE_ =========================
        {
            std::string out;
            for (const auto& parent : data.serializableParents)
            {
                out += std::format("\t\tvisitor(std::type_identity<{}>{{}});\n", parent);
            }
            FindAndReplaceAll(finalString, "@@F_FOR_EACH_BASE_", out);
            FindAndReplaceAll(finalString, "@@BASE_NUMBERS_",
                              std::to_string(data.serializableParents.size()));
        }

        // =================== F_SERIALIZE_ =========================
        {
            std::string out;
            out += R"(if (!noSignals)
        {
            _RTryCallPreSerialize<@@NAME_>(obj);
        }
)";

            for (const auto& parent : data.serializableParents)
            {
                out += "\n\t\ts.write("s + namespaceName.data() + "<" + parent
                       + ">::Serialize<RImpl>(obj).getData());";
            }

            for (const auto& field : data.fields)
            {
                std::string valueStr;
                if (field.typeMeta)
                {
                    valueStr = std::format("R<{}>::Serialize(obj.{}).getData()", field.type.name,
                                           field.name);
                }
                else
                {
                    valueStr = "obj." + field.name;
                }

                bool shouldPutElseKeyword = false;

                if ((field.flags & RPoint::SuppressLimits) == 0)
                {
                    if (!field.min.empty() || !field.max.empty())
                    {
                        out += "\n";
                    }

                    if (!field.min.empty())
                    {
                        out += "\n\t\t";
                        out += std::format("if (obj.{} < {}) {{", field.name, field.min);
                        out += std::format(" s.write(\"{}\", {}({}));", field.name, field.type.name,
                                           field.min);
                        out += " }";

                        shouldPutElseKeyword = true;
                    }

                    if (!field.max.empty())
                    {
                        out += "\n\t\t";
                        if (shouldPutElseKeyword)
                        {
                            out += "else ";
                        }
                        out += std::format("if (obj.{} > {}) {{", field.name, field.max);
                        out += std::format(" s.write(\"{}\", {}({}));", field.name, field.type.name,
                                           field.max);
                        out += " }";

                        shouldPutElseKeyword = true;
                    }
                }

                out += "\n\t\t";
                if (shouldPutElseKeyword)
                {
                    out += "else { ";
                }
                out += std::format("s.write(\"{}\", {});", field.name, valueStr);
                if (shouldPutElseKeyword)
                {
                    out += " }\n";
                }
            }

            out += R"(
        if (!noSignals)
        {
            _RTryCallPostSerialize<@@NAME_>(obj, s.logs());
        })";

            FindAndReplaceAll(finalString, "@@F_SERIALIZE_", out);
        }

        // =================== F_DESERIALIZE_ =========================
        {
            std::string out;

            out += R"(if (!noSignals)
        {
            _RTryCallPreDeserialize<@@NAME_>(obj);
        }
)";

            for (const auto& parent : data.serializableParents)
            {
                const char* ignoreSignalParam
                    = _config->ignoreSerializationSignals->value ? "true" : "false";
                out += "\n\t\t"s + namespaceName.data() + "<" + parent
                       + ">::Deserialize<RImpl>(s, obj, " + ignoreSignalParam + ");";
            }

            for (const auto& field : data.fields)
            {
                if (field.typeMeta && field.typeMeta->type == ContextType::EnumClass)
                {
                    out += "\n\t\t{";

                    out += "\n\t\t\tstd::string _readTmp;";

                    out += "\n\t\t\ts.read(\"" + field.name + "\", _readTmp";
                    if (!field.defaultValue.empty())
                    {
                        out += ", R<" + field.type.name + ">::ToString(" + field.defaultValue + ")";
                    }
                    out += ");";

                    // obj._type = R<AnimalType>::FromString(_type).value_or(AnimalType::None);
                    out += "\n\t\t\tobj." + field.name + " = R<" + field.type.name
                           + ">::FromString(_readTmp)";
                    if (!field.defaultValue.empty())
                    {
                        out += ".value_or(" + field.defaultValue + ")";
                    }
                    else
                    {
                        out += ".value()";
                    }
                    out += ";";

                    out += "\n\t\t}";
                }
                else
                {
                    std::string valueStr;
                    if (field.typeMeta)
                    {
                        valueStr = std::format("obj.{}", field.name);
                    }
                    else
                    {
                        valueStr = "obj." + field.name;
                    }

                    std::string defaultValueStr;
                    if (!field.defaultValue.empty())
                    {
                        defaultValueStr = field.defaultValue;
                    }
                    else
                    {
                        defaultValueStr = "decltype(obj." + field.name + "){}";
                    }

                    if ((field.flags & RPoint::SuppressLimits) == 0)
                    {
                        if (!field.min.empty() || !field.max.empty())
                        {
                            out += "\n";
                        }
                    }
                    std::string flagsStr = std::to_string(field.flags);
                    out += std::format("\n\t\ts.read(\"{}\", {}, {}, {});", field.name, valueStr,
                                       defaultValueStr, flagsStr);

                    if ((field.flags & RPoint::SuppressLimits) == 0)
                    {
                        if (!field.min.empty())
                        {
                            out += "\n\t\t";
                            out += std::format("if (obj.{} < {}) {{ obj.{} = {}({}); }}",
                                               field.name, field.min, field.name, field.type.name,
                                               field.min);
                        }

                        if (!field.max.empty())
                        {
                            out += "\n\t\t";
                            out += std::format("if (obj.{} > {}) {{ obj.{} = {}({}); }}",
                                               field.name, field.max, field.name, field.type.name,
                                               field.max);
                        }
                        if (!field.min.empty() || !field.max.empty())
                        {
                            out += "\n";
                        }
                    }
                }
            }

            out += R"(
        if (!noSignals)
        {
            _RTryCallPostDeserialize<@@NAME_>(obj, s.logs());
        })";
            FindAndReplaceAll(finalString, "@@F_DESERIALIZE_", out);
        }

        // =================== F_GET_FIELD_ =========================
        {
            std::string out;
            for (const auto& field : data.fields)
            {
                std::string constCastStr;
                if (field.type.isConst)
                {
                    constCastStr = "const_cast<";
                    constCastStr += field.type.name;
                    constCastStr += "*>";
                }

                // clang-format off
                out += std::format(R"(        if (std::strcmp(fieldName, "{}") == 0)
        {{
            onFound({}(&obj.{}), "{}");
            return true;
        }}
)",  field.name, constCastStr, field.name, field.type.getNameWithCV());
                // clang-format on
            }

            out += "\n\t\treturn false;";
            FindAndReplaceAll(finalString, "@@F_GET_FIELD_", out);
        }

        // Default find & replace
        FindAndReplaceAll(finalString, nameMark, data.fullNamePath());
        FindAndReplaceAll(finalString, onlyNameMark, data.name);
        FindAndReplaceAll(finalString, parentsMark, data.parentSpace);
        FindAndReplaceAll(finalString, namespaceMark, BaseReflector::namespaceName.data());
        FindAndReplaceAll(finalString, funcPrefMark, "static ");

        return finalString;
    }

} // namespace JRM
