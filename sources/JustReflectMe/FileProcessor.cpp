

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

#include "FileProcessor.h"

#include <filesystem>
#include <fstream>

namespace
{
    [[nodiscard]] std::string ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate);
        if (!file)
        {
            throw std::runtime_error("Cannot open a file: " + filename);
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer;
        buffer.reserve(size);
        buffer.resize(size);

        if (!file.read(buffer.data(), size)) [[unlikely]]
        {
            throw std::runtime_error("Error reading file");
        }

        return buffer;
    }
} // namespace

namespace JRM
{

    void FileProcessor::scanContent() const
    {
        const std::string content = getFileContent(_path);

        for (std::size_t i = 0; i < _reflectors.size(); ++i)
        {
            _reflectors[i]->scanContent(content);
        }
    }

    std::string FileProcessor::getFileContent(const std::string& filename) const
    {
        std::string content1 = ReadFile(filename);

        std::string content2;
        content2.reserve(content1.size());

        // Removing all block comments
        bool nowInsideBlockComment = false;
        for (std::size_t i = 1; i < content1.size() && content1[i]; ++i)
        {
            if (content1[i - 1] == '/' && content1[i] == '*')
            {
                if (nowInsideBlockComment) [[unlikely]]
                {
                    throw std::runtime_error(
                        "Can't parse a file: " + filename
                        + " - troubles with parsing block comment. File can be incorrect.");
                }
                nowInsideBlockComment = true;
                if (!content2.empty())
                {
                    content2.pop_back();
                }
            }

            if (content1[i - 1] == '*' && content1[i] == '/')
            {
                nowInsideBlockComment = false;
                continue;
            }

            if (!nowInsideBlockComment || content1[i] == '\n')
            {
                content2.push_back(content1[i]);
            }
        }
        content1.resize(0);

        // Removing all comments: //
        if (!content2.empty())
        {
            content1.push_back(content2.front());
        }
        bool nowComment = false;
        for (std::size_t i = 1; i < content2.size() && content2[i]; ++i)
        {
            if (nowComment)
            {
                if (content2[i] == '\n')
                {
                    nowComment = false;
                }
                else
                {
                    continue;
                }
            }

            if (content2[i] == '/' && content2[i - 1] == '/')
            {
                nowComment = true;
                content1.pop_back();
                continue;
            }

            content1.push_back(content2[i]);
        }
        content2.resize(0);

        // Removing all strings: "hello world"
        bool nowDoubleQuote = false;
        for (std::size_t i = 0; i < content1.size() && content1[i]; ++i)
        {
            if (nowDoubleQuote && content1[i] == '\n')
            {
                nowDoubleQuote = false;
            }

            if (content1[i] == '"')
            {
                nowDoubleQuote = !nowDoubleQuote;
                if (!nowDoubleQuote)
                {
                    continue;
                }
            }

            if (!nowDoubleQuote)
            {
                content2.push_back(content1[i]);
            }
        }
        content1.resize(0);

        // Removing all chars: 'c'
        bool nowSingleQuote = false;
        for (std::size_t i = 0; i < content2.size() && content2[i]; ++i)
        {
            if (content2[i] == '\'')
            {
                nowSingleQuote = !nowSingleQuote;
                if (!nowSingleQuote)
                {
                    continue;
                }
            }

            if (!nowSingleQuote)
            {
                content1.push_back(content2[i]);
            }
        }
        content2.resize(0);

        return content1;
    }

    std::pair<std::string, std::string> FileProcessor::generateFilenames() const
    {
        const auto extIndex = _path.find_last_of(".");
        if (extIndex == std::string::npos)
        {
            throw std::runtime_error("Can't find extension in file name: " + _path);
        }

        std::string headerPath = _path.substr(0, extIndex);
        headerPath += newFileExtension;
        std::string sourcePath = headerPath;
        headerPath += ".h";
        sourcePath += ".cpp";

        return { headerPath, sourcePath };
    }

    void FileProcessor::generateNewContent()
    {
        auto [hppPath, cppPath] = generateFilenames();

        for (const auto& reflector : _reflectors)
        {
            std::string hpp = reflector->generateHeaderFile(hppPath);
            std::string cpp = reflector->generateSourceFile(hppPath);
        }
    }

    void FileProcessor::run(const std::string& path)
    {
        _path = path;

        if (!std::filesystem::exists(_path))
        {
            throw std::runtime_error("File does not exist: '" + _path + "'");
        }

        scanContent();
        generateNewContent();
    }

} // namespace JRM