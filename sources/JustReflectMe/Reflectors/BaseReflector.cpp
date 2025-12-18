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

namespace JRM
{

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

        onScan();
    }

    std::string BaseReflector::generateHeaderFile(const std::string& newHeaderPath) const
    {
        std::string result;
        result.reserve(1024);

        result += warningCommentAtFileTop;

        result += "#pragma once\n\n";

        result += "namespace ";
        result += namespaceName;
        result += "\n{\n\n";

        result += onGenerateHeaderFile();

        result += "\n\n} // namespace\n";

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
        result += "\n{\n\n";

        result += onGenerateSourceFile();

        result += "\n\n} // namespace\n";

        return result;
    }

} // namespace JRM