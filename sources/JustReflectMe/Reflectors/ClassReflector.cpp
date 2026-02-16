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

    std::string ClassReflector::TokenData::fullNamePath() const
    {
        if (parentSpace.empty())
        {
            return name;
        }

        return parentSpace + "::" + name;
    }

    std::string ClassReflector::onGenerateHeaderFile(FileData& fileData, const Config& config) const
    {
        std::string result;
        result.reserve(1024);

        return result;
    }

    std::string ClassReflector::onGenerateSourceFile(FileData& fileData, const Config& config) const
    {
        std::string result;
        result.reserve(1024);

        return result;
    }

    void ClassReflector::onScan(const FileData& fileData)
    {
    }

    std::string ClassReflector::generateDeclaration(const TokenData& data,
                                                    const Config& config) const
    {
        std::string result;

        return result;
    }

    std::string ClassReflector::generateImplementation(const TokenData& data,
                                                       const Config& config) const
    {
        std::string result;

        return result;
    }

} // namespace JRM