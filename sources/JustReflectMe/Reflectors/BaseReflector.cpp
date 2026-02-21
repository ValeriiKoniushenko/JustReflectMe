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

#include "BaseReflector.h"

#include "../Config.h"
#include "../FileData.h"
#include "../Scopes.h"
#include "JustReflectMe/FileNavigationHelper.h"

#include <cstring>
#include <iostream>

using namespace FileNavigator;

namespace JRM
{

    SyntaxException::SyntaxException(const std::string& message, std::size_t indexInFile)
        : std::runtime_error(message),
          _indexInFile(indexInFile)
    {
    }

    std::string SyntaxException::getFullMessage(const std::string& content,
                                                const std::string& pathToFile) const
    {
        const auto pos = GetLineNumberAndColumn(content.c_str(), _indexInFile);
        std::string result = pathToFile + ":";
        result += std::to_string(pos.first) + ":" + std::to_string(pos.second) + ": error: ";
        result += what();
        return result;
    }

    bool BaseReflector::canProcessContent(const std::string& content) const
    {
        auto pos = content.find(getTriggerKeyword());
        if (pos == std::string::npos)
        {
            return false;
        }

        const auto len = getTriggerKeyword().size();
        return pos + len < content.size() - 1 && !std::isalnum(content[pos + len]);
    }

    void BaseReflector::scanContent(FileData& data)
    {
        const auto& content = data.getContent();

        auto pos = findTriggerKeyword(content, getTriggerKeyword(), 0);
        while (pos != std::string::npos)
        {
            _tokens.emplace_back(pos);
            pos = findTriggerKeyword(content, getTriggerKeyword(), pos + 1);
        }

        if (_tokens.empty())
        {
            return;
        }

        data.scanScopes();

        onScan(data);
    }

    std::string BaseReflector::generateHeaderFile(FileData& data, const Config& config) const
    {
        std::string result;
        result.reserve(1024);

        result += onGenerateHeaderFile(data, config);
        
        if (!result.empty() && result.back() != '\n')
        {
            result += '\n';
        }

        return result;
    }

    std::string BaseReflector::generateSourceFile(const std::string& newHeaderPath, FileData& data,
                                                  const Config& config) const
    {
        std::string result;
        result.reserve(1024);

        result += onGenerateSourceFile(data, config);

        if (!result.empty() && result.back() != '\n')
        {
            result += '\n';
        }

        return result;
    }

    std::set<std::string> BaseReflector::getIncludes() const
    {
        return { "string" };
    }

    void BaseReflector::setHasImplTranslationUnit(bool val) noexcept
    {
        if (_isSupportImplTranslationUnit)
        {
            _hasImplTranslationUnit = val;
        }
    }

    bool BaseReflector::hasImplTranslationUnit() const noexcept
    {
        return _isSupportImplTranslationUnit && _hasImplTranslationUnit;
    }

    bool BaseReflector::isSupportImplTranslationUnit() const noexcept
    {
        return _isSupportImplTranslationUnit;
    }

    void BaseReflector::TokenEntry::requireValidTokenBasedOnContent(
        const std::string& content) const
    {
        if (!isValid()) [[unlikely]]
        {
            throw std::runtime_error("Invalid token was found.");
        }

        if (begin >= content.size()) [[unlikely]]
        {
            throw std::runtime_error("Token begin position is out of range: "
                                     + std::to_string(begin)
                                     + " But content length is: " + std::to_string(content.size()));
        }
    }

    std::string BaseReflector::BaseTokenData::fullNamePath() const
    {
        if (parentSpace.empty())
        {
            return name;
        }

        return parentSpace + "::" + name;
    }

    void BaseReflector::WarnMessage(const char* source, std::size_t indexInFileWithError,
                                    const std::string& filepath, const std::string& errorMessage)
    {
        const auto pos = GetLineNumberAndColumn(source, indexInFileWithError);
        std::string result;
        result.reserve(192);
        result = filepath;
        result += ":";
        result += std::to_string(pos.first) + ":" + std::to_string(pos.second) + ": warning: ";
        result += errorMessage;

        std::cout << result << "\n";
    }

    std::string BaseReflector::PrettyPrintScope(const Scope* scope)
    {
        std::string out;

        while (scope)
        {
            out.insert(0, scope->getIdentifier());
            scope = scope->parent;
            if (scope && scope->parent)
            {
                out.insert(0, "::");
            }
        }

        return out;
    }

    std::string BaseReflector::PrettyPrintIdentifier(const Scope* scope)
    {
        std::string out;

        out += Scope::ToString(scope->type);
        out += " ";
        out += PrettyPrintScope(scope);

        return out;
    }

    std::size_t BaseReflector::findTriggerKeyword(const std::string& content,
                                                  std::string_view keyword, std::size_t offset)
    {
        if (const auto pos = content.find(keyword, offset); pos != std::string::npos)
        {
            return IsWord(content, keyword, pos) ? pos : std::string::npos;
        }

        return std::string::npos;
    }

} // namespace JRM
