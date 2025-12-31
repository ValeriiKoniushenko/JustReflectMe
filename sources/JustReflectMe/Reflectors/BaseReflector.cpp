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

#include "BaseReflector.h"

#include "JustReflectMe/FileNavigationHelper.h"

using namespace FileNavigator;

namespace Reflect::HelloWorld
{

    // clang-format off
    const std::string& ToString(::HelloWorld value)
    {
        const auto& data = Reflect::HelloWorld::ToMapCN();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        static constexpr std::string empty{};
        return empty;
    }

    std::optional<::HelloWorld> FromString(const std::string& value)
    {
        const auto& data = Reflect::HelloWorld::ToMapNC();
        const auto it = data.find(value);
        if (it != data.end()) [[likely]]
        {
            return it->second;
        }
        return std::nullopt;

    }

    const std::array<::HelloWorld, 2>& ToArrayC()
    {
        static constexpr std::array constants = {
            ::HelloWorld::Hello,
            ::HelloWorld::World
        };

        return constants;
    }

    const std::array<std::string, 2>& ToArrayN()
    {
        static constexpr std::array names = {
            std::string("Hello"),
            std::string("World")
        };

        return names;
    }

    const std::unordered_map<::HelloWorld, std::string>& ToMapCN()
    {
        static const std::unordered_map<::HelloWorld, std::string> map = {
            { ::HelloWorld::Hello, "Hello" },
            { ::HelloWorld::World, "World" }
        };

        return map;
    }

    const std::unordered_map<std::string, ::HelloWorld>& ToMapNC()
    {
        static const std::unordered_map<std::string, ::HelloWorld> map = {
            { "Hello", ::HelloWorld::Hello },
            { "World", ::HelloWorld::World }
        };

        return map;
    }
    // clang-format on

} // namespace Reflect::HelloWorld

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

    void BaseReflector::scanContent(const std::string& content)
    {
        const char* triggerKeyword = getTriggerKeyword();

        auto pos = content.find(triggerKeyword);
        while (pos != std::string::npos)
        {
            _tokens.emplace_back(pos);
            pos = content.find(triggerKeyword, pos + 1);
        }

        onScan(content);
    }

    std::string BaseReflector::generateHeaderFile(const std::string& newHeaderPath) const
    {
        std::string result;
        result.reserve(1024);

        result += warningCommentAtFileTop;

        result += "#pragma once\n\n";

        result += onGenerateHeaderFilePreNamespace();

        result += "namespace ";
        result += namespaceName;
        result += "\n{\n";

        result += onGenerateHeaderFile();

        result += "\n} // namespace\n";

        return result;
    }

    std::string BaseReflector::generateSourceFile(const std::string& newHeaderPath) const
    {
        std::string result;
        result.reserve(1024);

        result += warningCommentAtFileTop;
        result += "#include \"" + newHeaderPath + "\"\n\n";

        result += "namespace ";
        result += namespaceName;
        result += "\n{\n";

        result += onGenerateSourceFile();

        result += "\n\n} // namespace\n";

        return result;
    }

} // namespace JRM