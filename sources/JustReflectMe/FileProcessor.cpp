

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
        std::ifstream file(filename);
        if (!file)
        {
            throw std::runtime_error("Cannot open file");
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

    bool FileProcessor::TokenEntry::isValid() const noexcept
    {
        return begin != invalidPosition && end != invalidPosition && begin < end
               && processableReflectorTypeHash != 0;
    }

    std::vector<FileProcessor::TokenEntry> FileProcessor::findAllEntryPoints() const
    {
        std::string fileContent = ReadFile(_filePath);

        std::vector<FileProcessor::TokenEntry> out;

        return out;
    }

    void FileProcessor::setFilePath(const std::string& path)
    {
        _filePath = path;
    }

    void FileProcessor::run()
    {
        if (!std::filesystem::exists(_filePath))
        {
            throw std::runtime_error("File does not exist: '" + _filePath + "'");
        }


    }

} // namespace JRM