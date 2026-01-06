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

#include "../FileData.h"
#include "../Scopes.h"
#include "JustReflectMe/FileNavigationHelper.h"

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
        return content.find(getTriggerKeyword());
    }

    void BaseReflector::scanContent(FileData& data)
    {
        const char* triggerKeyword = getTriggerKeyword();

        const auto& content = data.getContent();

        auto pos = content.find(triggerKeyword);
        while (pos != std::string::npos)
        {
            _tokens.emplace_back(pos);
            pos = content.find(triggerKeyword, pos + 1);
        }

        if (_tokens.empty())
        {
            return;
        }

        data.scanScopes();

        onScan(data);
    }

    std::string BaseReflector::generateHeaderFile(const std::string& newHeaderPath, FileData& data) const
    {
        std::string result;
        result.reserve(1024);

        result += warningCommentAtFileTop;

        if (hasSeparateTranslationUnit())
        {
            result += "#pragma once\n\n";
        }

        result += onGenerateHeaderFilePreNamespace(data);

        result += "namespace ";
        result += namespaceName;
        result += "\n{\n";

        result += onGenerateHeaderFile(data);

        result += "\n} // namespace\n";

        return result;
    }

    std::string BaseReflector::generateSourceFile(const std::string& newHeaderPath, FileData& data) const
    {
        if (_type == ImplType::InlOnly)
        {
            return {};
        }

        std::string result;
        result.reserve(1024);

        result += warningCommentAtFileTop;
        result += "#include \"" + newHeaderPath + "\"\n\n";

        result += "namespace ";
        result += namespaceName;
        result += "\n{\n";

        result += onGenerateSourceFile(data);

        result += "\n\n} // namespace\n";

        return result;
    }

    bool BaseReflector::hasSeparateTranslationUnit() const noexcept
    {
        return _type == ImplType::HeaderCpp;
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

} // namespace JRM